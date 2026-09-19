#include "skin.h"
#include <windowsx.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cwctype>
#include <sstream>
#include <stdexcept>

namespace waskin {
namespace {
constexpr UINT_PTR subclassId=0x77534b49, timerId=0x77534b49;
constexpr int hitShade=500, hitDrag=501, hitResize=502, hitShuffle=503, hitRepeat=504,
    hitSeek=505, hitVolume=506, hitBalance=507, hitRow=1000, hitScroll=508;
std::string Trim(std::string s) {
    const auto first=s.find_first_not_of(" \t\r\n");
    if(first==std::string::npos) return {};
    return s.substr(first,s.find_last_not_of(" \t\r\n")-first+1);
}
std::string Lower(std::string s) { for(char& c:s) if(c>='A'&&c<='Z') c=char(c+32); return s; }
void ReadIni(const Bytes& bytes,const std::string& prefix,std::unordered_map<std::string,std::string>& values) {
    std::istringstream stream(std::string(bytes.begin(),bytes.end())); std::string line, section;
    while(std::getline(stream,line)) {
        line=Trim(line); if(line.empty() || line[0]==';' || line[0]=='#') continue;
        if(line[0]=='[') { const auto end=line.find(']'); if(end!=std::string::npos) section=Lower(Trim(line.substr(1,end-1))); }
        else { const auto eq=line.find('='); if(eq!=std::string::npos) values[prefix+section+"/"+Lower(Trim(line.substr(0,eq)))]=Trim(line.substr(eq+1)); }
    }
}
COLORREF Color(const std::string& text,COLORREF fallback) {
    auto s=Trim(text); if(!s.empty() && s[0]=='#') s.erase(0,1);
    if(s.size()!=6) return fallback;
    for(char c:s) if(!std::isxdigit(static_cast<unsigned char>(c))) return fallback;
    const auto n=std::stoul(s,nullptr,16); return RGB((n>>16)&255,(n>>8)&255,n&255);
}
void Fill(HDC dc,RECT rect,COLORREF color) {
    SetDCBrushColor(dc,color); FillRect(dc,&rect,static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
}
bool Inside(POINT p,int x,int y,int w,int h) { return p.x>=x && p.x<x+w && p.y>=y && p.y<y+h; }
std::wstring Time(int64_t milliseconds) {
    const auto seconds=std::max<int64_t>(0,milliseconds/1000);
    wchar_t text[48]{}; swprintf_s(text,L"%lld:%02lld",seconds/60,seconds%60); return text;
}
}

Image::Image(const Bytes& bytes) {
    if(bytes.empty()) return;
    if(bytes.size()<sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)) throw std::runtime_error("Invalid BMP");
    BITMAPFILEHEADER file{}; BITMAPINFOHEADER info{};
    memcpy(&file,bytes.data(),sizeof(file)); memcpy(&info,bytes.data()+sizeof(file),sizeof(info));
    if(file.bfType!=0x4d42 || info.biSize<40 || info.biSize>124 || info.biWidth<=0 || info.biWidth>4096 ||
       info.biHeight==0 || info.biHeight < -4096 || info.biHeight>4096 || info.biPlanes!=1 ||
       (info.biBitCount!=1 && info.biBitCount!=4 && info.biBitCount!=8 && info.biBitCount!=16 && info.biBitCount!=24 && info.biBitCount!=32) ||
       (info.biCompression!=BI_RGB && info.biCompression!=BI_BITFIELDS && info.biCompression!=BI_RLE8 && info.biCompression!=BI_RLE4)) throw std::runtime_error("Unsupported BMP");
    width=info.biWidth; height=std::abs(info.biHeight);
    const size_t palette=info.biBitCount<=8 ? (info.biClrUsed?info.biClrUsed:1u<<info.biBitCount) : 0;
    if(palette>256) throw std::runtime_error("Invalid BMP palette");
    const size_t header=sizeof(file)+info.biSize+palette*4+((info.biCompression==BI_BITFIELDS && info.biSize==40)?12:0);
    const size_t stride=((size_t(width)*info.biBitCount+31)/32)*4;
    const bool rle=info.biCompression==BI_RLE8 || info.biCompression==BI_RLE4;
    if(header>bytes.size() || file.bfOffBits<header || file.bfOffBits>bytes.size() || (!rle && stride*height>bytes.size()-file.bfOffBits))
        throw std::runtime_error("Truncated BMP");
    Bytes decoded;
    if(rle) {
        if(info.biHeight<0 || (info.biCompression==BI_RLE8 && info.biBitCount!=8) ||
           (info.biCompression==BI_RLE4 && info.biBitCount!=4)) throw std::runtime_error("Invalid BMP RLE format");
        decoded.resize(stride*height);
        size_t input=file.bfOffBits; int x=0,y=0;bool ended=false;
        const auto byte=[&]() {if(input==bytes.size()) throw std::runtime_error("Truncated BMP RLE");return bytes[input++];};
        const auto pixel=[&](unsigned value) {
            if(x>=width || y>=height || value>=palette) throw std::runtime_error("BMP RLE outside image");
            auto& target=decoded[size_t(y)*stride+(info.biBitCount==8?x:x/2)];
            if(info.biBitCount==8) target=uint8_t(value);
            else target=uint8_t((x%2)?(target&0xf0)|value:(target&0x0f)|(value<<4));
            ++x;
        };
        while(!ended) {
            const unsigned count=byte(),value=byte();
            if(count) for(unsigned i=0;i<count;++i) pixel(info.biBitCount==8?value:(i%2?value&15:value>>4));
            else if(value==0) {x=0;if(++y>height) throw std::runtime_error("BMP RLE excess rows");}
            else if(value==1) ended=true;
            else if(value==2) {x+=byte();y+=byte();if(x>width || y>=height) throw std::runtime_error("BMP RLE delta outside image");}
            else {
                unsigned pair=0;
                for(unsigned i=0;i<value;++i) {
                    if(info.biBitCount==8) pixel(byte());
                    else {if(i%2==0) pair=byte();pixel(i%2?pair&15:pair>>4);}
                }
                const unsigned length=info.biBitCount==8?value:(value+1)/2;
                if(length%2) byte();
            }
        }
    }
    // Aligned BITMAPINFO storage also supports palette entries and bit masks.
    std::vector<DWORD> metadata((header-sizeof(file)+3)/4);
    memcpy(metadata.data(),bytes.data()+sizeof(file),header-sizeof(file));
    if(rle) {
        auto* uncompressed=reinterpret_cast<BITMAPINFOHEADER*>(metadata.data());
        uncompressed->biCompression=BI_RGB;uncompressed->biSizeImage=DWORD(decoded.size());
    }
    HDC screen=GetDC(nullptr);
    bitmap=CreateDIBitmap(screen,reinterpret_cast<const BITMAPINFOHEADER*>(metadata.data()),CBM_INIT,
        rle?decoded.data():bytes.data()+file.bfOffBits,reinterpret_cast<const BITMAPINFO*>(metadata.data()),DIB_RGB_COLORS);
    ReleaseDC(nullptr,screen);
    if(!bitmap) throw std::runtime_error("Cannot create BMP");
}
Skin::Skin(const wchar_t* path,const TtpSkinHost* host) {
    if(host) {
        // Old v1 hosts end at command. Never read a partially supplied tail.
        std::memcpy(&host_,host,TTP_SKIN_HOST_V1_SIZE);
        if(host->size>=sizeof(TtpSkinHost)) host_.drag=host->drag;
    }
    Archive archive(path);
    for(const auto* name:{"main.bmp","cbuttons.bmp","titlebar.bmp","shufrep.bmp","posbar.bmp","volume.bmp","balance.bmp",
        "numbers.bmp","nums_ex.bmp","text.bmp","playpaus.bmp","monoster.bmp","eqmain.bmp","eq_ex.bmp","pledit.bmp"}) {
        if(archive.Has(name)) images_.emplace(name,Image(archive.Read(name)));
    }
    const auto main=images_.find("main.bmp");
    if(main==images_.end() || main->second.width<275 || main->second.height<116) throw std::runtime_error("Missing classic main.bmp");
    ReadIni(archive.Read("pledit.txt"),"playlist/",ini_);
    ReadIni(archive.Read("region.txt"),"region/",ini_);
    normal_=Color(ini_["playlist/text/normal"],normal_);
    current_=Color(ini_["playlist/text/current"],current_);
    background_=Color(ini_["playlist/text/normalbg"],background_);
    selection_=Color(ini_["playlist/text/selectedbg"],selection_);
    font_=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
        NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Tahoma");
    for(int i=0;i<3;++i) { views_[i].skin=this; views_[i].kind=i; }
}
Skin::~Skin() { Detach(); if(font_) DeleteObject(font_); }
TtpSkinState Skin::State() const {
    TtpSkinState state{}; state.size=sizeof(state); state.volume=100; state.elapsed=1; state.playing_row=-1;
    wcscpy_s(state.title,L"TTPlayer");
    if(host_.query) host_.query(host_.context,&state);
    return state;
}
void Skin::Command(uint32_t command,int32_t value) const { if(host_.command) host_.command(host_.context,command,value); }
bool Skin::Blit(HDC dc,const char* name,int x,int y,int w,int h,int sx,int sy,int sw,int sh) const {
    auto found=images_.find(name); if(found==images_.end() || !found->second.bitmap) return false;
    const auto& image=found->second; if(!sw) sw=w; if(!sh) sh=h;
    if(sx<0 || sy<0 || sw<=0 || sh<=0 || sx+sw>image.width || sy+sh>image.height) return false;
    HDC source=CreateCompatibleDC(dc); if(!source) return false;
    const auto old=SelectObject(source,image.bitmap);
    SetStretchBltMode(dc,COLORONCOLOR);
    const BOOL result=StretchBlt(dc,x,y,w,h,source,sx,sy,sw,sh,SRCCOPY);
    SelectObject(source,old); DeleteDC(source); return result!=FALSE;
}
void Skin::Text(HDC dc,RECT bounds,const std::wstring& text,COLORREF color,bool bitmap) const {
    const auto image=images_.find("text.bmp");
    const bool ascii=std::all_of(text.begin(),text.end(),[](wchar_t c){return c>=32 && c<127;});
    if(bitmap && ascii && image!=images_.end()) {
        const int saved=SaveDC(dc); IntersectClipRect(dc,bounds.left,bounds.top,bounds.right,bounds.bottom);
        int x=bounds.left;
        // Classic text.bmp: alphabet in row 0; digits and punctuation in row 1.
        const std::wstring row1=L"0123456789.:()-'!_+\\/[]^&%,=$#";
        for(wchar_t c:text) {
            c=wchar_t(towupper(c)); int sx=142,sy=0;
            if(c>=L'A'&&c<=L'Z') sx=(c-L'A')*5;
            else { const auto at=row1.find(c); if(at!=std::wstring::npos) {sx=int(at)*5;sy=6;} }
            Blit(dc,"text.bmp",x,bounds.top,5,6,sx,sy); x+=5;
            if(x>=bounds.right) break;
        }
        RestoreDC(dc,saved); return;
    }
    const auto old=SelectObject(dc,font_?font_:GetStockObject(DEFAULT_GUI_FONT));
    SetBkMode(dc,TRANSPARENT); SetTextColor(dc,color);
    DrawTextW(dc,text.c_str(),int(text.size()),&bounds,DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
    SelectObject(dc,old);
}
void Skin::DrawMain(View& view,HDC dc,const TtpSkinState& s) {
    Blit(dc,"main.bmp",0,0,275,116);
    const bool active=GetActiveWindow()==view.window;
    Blit(dc,"titlebar.bmp",0,0,275,14,27,view.shaded?(active?29:42):(active?0:15));
    if(view.shaded) {
        Text(dc,{5,3,125,12},s.title,current_,true);
        Text(dc,{130,2,190,13},Time(s.position_ms),current_);
        return;
    }
    for(int i=0;i<5;++i) {
        static constexpr int commands[]={TTP_SKIN_PREVIOUS,TTP_SKIN_PLAY,TTP_SKIN_PAUSE,TTP_SKIN_STOP,TTP_SKIN_NEXT};
        if(!Blit(dc,"cbuttons.bmp",16+i*23,88,i==4?22:23,18,i*23,view.pressed==commands[i]?18:0)) {
            static const wchar_t* labels[]={L"|<",L">",L"||",L"[]",L">|"};
            Text(dc,{16+i*23,88,38+i*23,106},labels[i],current_);
        }
    }
    if(!Blit(dc,"cbuttons.bmp",136,89,22,16,114,view.pressed==TTP_SKIN_OPEN?16:0)) Text(dc,{136,89,158,106},L"+",current_);
    Blit(dc,"titlebar.bmp",6,3,9,9,0,view.pressed==TTP_SKIN_MENU?9:0);
    Blit(dc,"titlebar.bmp",244,3,9,9,9,view.pressed==TTP_SKIN_MINIMIZE?9:0);
    Blit(dc,"titlebar.bmp",254,3,9,9,view.pressed==hitShade?9:0,18);
    Blit(dc,"titlebar.bmp",264,3,9,9,18,view.pressed==TTP_SKIN_CLOSE?9:0);
    Blit(dc,"shufrep.bmp",164,89,47,15,28,(s.mode==4?30:0)+(view.pressed==hitShuffle?15:0));
    Blit(dc,"shufrep.bmp",210,89,28,15,0,(s.mode==1||s.mode==3?30:0)+(view.pressed==hitRepeat?15:0));
    Blit(dc,"shufrep.bmp",219,58,23,12,view.pressed==TTP_SKIN_EQUALIZER?46:0,61+(s.equalizer_visible?12:0));
    Blit(dc,"shufrep.bmp",242,58,23,12,23+(view.pressed==TTP_SKIN_PLAYLIST?46:0),61+(s.playlist_visible?12:0));
    const int volume=std::clamp(s.volume,0,100), balance=std::clamp(s.balance,-100,100);
    Blit(dc,"volume.bmp",107,57,68,13,0,(volume*27/100)*15);
    Blit(dc,"volume.bmp",107+volume*51/100,58,14,11,view.pressed==hitVolume?0:15,422);
    const char* pan=images_.contains("balance.bmp")?"balance.bmp":"volume.bmp";
    Blit(dc,pan,177,57,38,13,9,(std::abs(balance)*27/100)*15);
    Blit(dc,pan,177+(balance+100)*24/200,58,14,11,view.pressed==hitBalance?0:15,422);
    Blit(dc,"posbar.bmp",16,72,248,10);
    if(s.duration_ms>0) {
        const int pos=int(std::clamp<int64_t>(s.position_ms,0,s.duration_ms)*219/s.duration_ms);
        Blit(dc,"posbar.bmp",16+pos,72,29,10,view.pressed==hitSeek?278:248,0);
    }
    int64_t time=s.elapsed?s.position_ms:std::max<int64_t>(0,s.duration_ms-s.position_ms);
    int seconds=int(std::max<int64_t>(0,time/1000));
    const char* numbers=images_.contains("nums_ex.bmp")?"nums_ex.bmp":"numbers.bmp";
    const int digits[]={seconds/600%10,seconds/60%10,seconds/10%6,seconds%10};
    const int places[]={48,60,78,90};
    for(int i=0;i<4;++i) if(!Blit(dc,numbers,places[i],26,9,13,digits[i]*9,0)) {
        Text(dc,{places[i],26,places[i]+9,39},std::to_wstring(digits[i]),current_);
    }
    if(!s.elapsed) Text(dc,{37,26,47,39},L"-",current_);
    const int icon=s.playback==2?0:s.playback==3?9:18;
    Blit(dc,"playpaus.bmp",26,28,9,9,icon,0);
    Blit(dc,"monoster.bmp",212,41,28,12,29,s.channels==1?0:12);
    Blit(dc,"monoster.bmp",239,41,29,12,0,s.channels>=2?0:12);
    std::wstring title=s.title;
    if(title.size()>30) { title+=L"   ***   "; const auto start=(ticks_/4)%title.size(); title=title.substr(start)+title.substr(0,start); }
    Text(dc,{111,24,266,33},title,current_,true);
    Text(dc,{111,40,131,49},s.bitrate>0?std::to_wstring(s.bitrate/1000):L"",current_,true);
    Text(dc,{156,40,171,49},s.sample_rate>0?std::to_wstring(s.sample_rate/1000):L"",current_,true);
}
void Skin::DrawPlaylist(View& view,HDC dc,int width,int height,const TtpSkinState& s) {
    Fill(dc,{0,0,width,height},background_);
    const int state=GetActiveWindow()==view.window?0:21;
    if(view.shaded) {
        Blit(dc,"pledit.bmp",0,0,25,14,72,42);
        Blit(dc,"pledit.bmp",25,0,width-75,14,72,57,25,14);
        Blit(dc,"pledit.bmp",width-50,0,50,14,99,state?57:42);
        Text(dc,{6,0,width-40,14},s.title,current_); return;
    }
    const auto tile=[&](int x,int y,int w,int h,int sx,int sy,int tw,int th) {
        for(int yy=0;yy<h;yy+=th) for(int xx=0;xx<w;xx+=tw)
            Blit(dc,"pledit.bmp",x+xx,y+yy,std::min(tw,w-xx),std::min(th,h-yy),sx,sy);
    };
    Blit(dc,"pledit.bmp",0,0,25,20,0,state);
    tile(25,0,(width-100)/2-25,20,127,state,25,20);
    Blit(dc,"pledit.bmp",(width-100)/2,0,100,20,26,state);
    tile((width+100)/2,0,width-25-(width+100)/2,20,127,state,25,20);
    Blit(dc,"pledit.bmp",width-25,0,25,20,153,state);
    tile(0,20,12,height-58,0,42,12,29);
    tile(width-20,20,20,height-58,31,42,20,29);
    Blit(dc,"pledit.bmp",0,height-38,125,38,0,72);
    if(width>275) Blit(dc,"pledit.bmp",125,height-38,width-275,38,179,0,25,38);
    Blit(dc,"pledit.bmp",width-150,height-38,150,38,126,72);
    const int rows=std::max(1,(height-58)/13);
    const int maximum=std::max(0,int(s.track_count)-rows);
    view.scroll=std::clamp(view.scroll,0,maximum);
    for(int i=0;i<rows && uint32_t(view.scroll+i)<s.track_count;++i) {
        const int index=view.scroll+i, y=20+i*13;
        if(index==view.selected) Fill(dc,{12,y,width-20,y+13},selection_);
        TtpSkinTrack track{}; track.size=sizeof(track);
        if(host_.track && host_.track(host_.context,uint32_t(index),&track)) {
            const auto color=index==s.playing_row?current_:normal_;
            Text(dc,{13,y,width-57,y+13},std::to_wstring(index+1)+L". "+track.title,color);
            if(track.duration_ms>=0) Text(dc,{width-55,y,width-20,y+13},Time(track.duration_ms),color);
        }
    }
    const int travel=std::max(0,height-76);
    Blit(dc,"pledit.bmp",width-15,20+(maximum?view.scroll*travel/maximum:0),8,18,52,53);
    Text(dc,{width-145,height-28,width-25,height-16},std::to_wstring(s.track_count)+L" tracks",normal_);
}
void Skin::DrawEqualizer(View& view,HDC dc,const TtpSkinState& s) {
    if(!Blit(dc,"eqmain.bmp",0,0,275,116)) {
        Fill(dc,{0,0,275,116},RGB(35,35,35)); Text(dc,{8,0,250,14},L"Equalizer",current_);
    }
    const bool active=GetActiveWindow()==view.window;
    if(view.shaded) { Blit(dc,"eq_ex.bmp",0,0,275,14,0,active?0:15); return; }
    Blit(dc,"eqmain.bmp",0,0,275,14,0,active?134:149);
    Blit(dc,"eqmain.bmp",14,18,25,12,10+(s.eq_enabled?59:0)+(view.pressed==TTP_SKIN_EQ_ENABLE?118:0),119);
    Blit(dc,"eqmain.bmp",39,18,33,12,35,119);
    Blit(dc,"eqmain.bmp",217,18,44,12,224,view.pressed==TTP_SKIN_EQ_PRESETS?176:164);
    for(int i=0;i<11;++i) {
        const int x=i?78+(i-1)*18:21;
        const int pos=(12-std::clamp(s.eq[i],-12,12))*63/24;
        const int frame=27-pos*28/64;
        if(!Blit(dc,"eqmain.bmp",x,38,14,63,13+(frame%14)*15,frame<14?164:229)) Fill(dc,{x,38,x+14,101},RGB(10,10,10));
        if(!Blit(dc,"eqmain.bmp",x+1,39+pos*51/63,11,11,0,view.pressed==TTP_SKIN_EQ_VALUE+i?176:164))
            Fill(dc,{x+1,39+pos*51/63,x+12,50+pos*51/63},RGB(180,180,180));
    }
}
void Skin::Draw(View& view,HDC dc,int width,int height) {
    const int saved=SaveDC(dc); IntersectClipRect(dc,0,0,width,height);
    Fill(dc,{0,0,width,height},background_);
    const auto state=State();
    if(view.kind==0) DrawMain(view,dc,state);
    else if(view.kind==1) DrawPlaylist(view,dc,width,height,state);
    else DrawEqualizer(view,dc,state);
    RestoreDC(dc,saved);
}
void Skin::Paint(HWND window,HDC dc) {
    for(auto& view:views_) if(view.window==window) {
        RECT rect{}; GetClientRect(window,&rect);
        if(IsIconic(window)) GetClipBox(dc,&rect);
        HDC memory=CreateCompatibleDC(dc); HBITMAP bitmap=CreateCompatibleBitmap(dc,std::max(1L,rect.right),std::max(1L,rect.bottom));
        if(!memory || !bitmap) { if(memory) DeleteDC(memory); if(bitmap) DeleteObject(bitmap); return; }
        const auto old=SelectObject(memory,bitmap); Draw(view,memory,rect.right,rect.bottom);
        BitBlt(dc,0,0,rect.right,rect.bottom,memory,0,0,SRCCOPY);
        SelectObject(memory,old); DeleteObject(bitmap); DeleteDC(memory); return;
    }
}
HBITMAP Skin::Preview() {
    HDC screen=GetDC(nullptr), dc=CreateCompatibleDC(screen);
    HBITMAP result=CreateCompatibleBitmap(screen,275,116); ReleaseDC(nullptr,screen);
    if(!dc || !result) { if(dc) DeleteDC(dc); if(result) DeleteObject(result); return nullptr; }
    const auto old=SelectObject(dc,result); View view{}; Draw(view,dc,275,116);
    SelectObject(dc,old); DeleteDC(dc); return result;
}
void Skin::Region(View& view) {
    const char* names[]={"normal","playlist","equalizer"};
    std::string name=names[view.kind];
    if(view.shaded) name=view.kind==0?"windowshade":name+"ws";
    const auto key="region/"+name+"/";
    const auto counts=ini_.find(key+"numpoints"), points=ini_.find(key+"pointlist");
    HRGN region=nullptr;
    if(counts!=ini_.end() && points!=ini_.end()) {
        auto c=counts->second,p=points->second; std::replace(c.begin(),c.end(),',',' '); std::replace(p.begin(),p.end(),',',' ');
        std::istringstream cs(c),ps(p); std::vector<int> sizes; std::vector<POINT> coords; int n,total=0;
        while(cs>>n) { if(n<3 || n>8192-total) {sizes.clear();break;} sizes.push_back(n);total+=n; }
        POINT point{};
        while(ps>>point.x>>point.y) { if(std::abs(int64_t(point.x))>16384 || std::abs(int64_t(point.y))>16384 || coords.size()>=8192) {coords.clear();break;} coords.push_back(point); }
        if(!sizes.empty() && coords.size()==size_t(total)) region=CreatePolyPolygonRgn(coords.data(),sizes.data(),int(sizes.size()),WINDING);
    }
    if(!region) { RECT r{};GetClientRect(view.window,&r);region=CreateRectRgn(0,0,r.right,r.bottom); }
    if(!SetWindowRgn(view.window,region,TRUE) && region) DeleteObject(region);
}
void Skin::HideChildren(View& view) {
    for(HWND child=GetWindow(view.window,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)) {
        if(std::none_of(view.children.begin(),view.children.end(),[child](const auto& p){return p.first==child;}))
            view.children.emplace_back(child,(GetWindowLongPtrW(child,GWL_STYLE)&WS_VISIBLE)!=0);
        if(GetWindowLongPtrW(child,GWL_STYLE)&WS_VISIBLE) ShowWindow(child,SW_HIDE);
    }
}
HRESULT Skin::Attach(const TtpSkinWindows& windows) {
    Detach(); const HWND handles[]={windows.player,windows.playlist,windows.equalizer};
    RECT origin{}; GetWindowRect(windows.player,&origin);
    for(int i=0;i<3;++i) {
        if(!IsWindow(handles[i])) { if(i==0) {Detach();return E_INVALIDARG;} continue; }
        auto& v=views_[i]; v.window=handles[i]; GetWindowRect(v.window,&v.saved);
        v.saved_region=CreateRectRgn(0,0,0,0);
        if(GetWindowRgn(v.window,v.saved_region)==ERROR) {DeleteObject(v.saved_region);v.saved_region=nullptr;}
        if(!SetWindowSubclass(v.window,Subclass,subclassId,reinterpret_cast<DWORD_PTR>(&v))) {Detach();return E_FAIL;}
        HideChildren(v);
        const int height=i==1?232:116;
        SetWindowPos(v.window,nullptr,origin.left,origin.top+(i==1?232:i==2?116:0),275,height,SWP_NOACTIVATE|SWP_NOZORDER);
        v.expanded_height=height; Region(v);
        SetTimer(v.window,timerId,100,nullptr);
        InvalidateRect(v.window,nullptr,FALSE);
    }
    return S_OK;
}
void Skin::Detach() noexcept {
    for(auto& v:views_) {
        if(IsWindow(v.window)) {
            KillTimer(v.window,timerId);
            EndDrag(v);
            if(GetCapture()==v.window) ReleaseCapture();
            RemoveWindowSubclass(v.window,Subclass,subclassId);
            for(const auto& [child,visible]:v.children) if(IsWindow(child) && visible) ShowWindow(child,SW_SHOWNA);
            SetWindowPos(v.window,nullptr,0,0,v.saved.right-v.saved.left,v.saved.bottom-v.saved.top,SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
            if(SetWindowRgn(v.window,v.saved_region,TRUE)) v.saved_region=nullptr;
            InvalidateRect(v.window,nullptr,TRUE);
        }
        if(v.saved_region) DeleteObject(v.saved_region);
        v.saved_region=nullptr;v.window=nullptr;v.children.clear();v.pressed=0;v.dragging=v.resizing=v.host_drag=false;
    }
}
void Skin::ToggleShade(View& v) {
    RECT r{};GetClientRect(v.window,&r);
    if(!v.shaded) v.expanded_height=r.bottom;
    v.shaded=!v.shaded;
    SetWindowPos(v.window,nullptr,0,0,r.right,v.shaded?14:v.expanded_height,SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
    Region(v);InvalidateRect(v.window,nullptr,FALSE);
}
void Skin::Shade() { if(views_[0].window) ToggleShade(views_[0]); }
bool Skin::Translate(const MSG& message) {
    if(message.message!=WM_MOUSEWHEEL || !message.hwnd || GetCapture() ||
       !IsWindowEnabled(views_[0].window) || GetWindowThreadProcessId(message.hwnd,nullptr)!=GetCurrentThreadId()) return false;
    const POINT point{GET_X_LPARAM(message.lParam),GET_Y_LPARAM(message.lParam)};
    const HWND hit=WindowFromPoint(point);
    for(const auto& view:views_) {
        if(view.window && hit==view.window && message.hwnd!=view.window && IsWindowVisible(view.window) && IsWindowEnabled(view.window)) {
            SendMessageW(view.window,message.message,message.wParam,message.lParam);return true;
        }
    }
    return false;
}
int Skin::Hit(const View& v,POINT p) const {
    RECT r{};GetClientRect(v.window,&r);
    if(p.y<14) {
        if(p.x>=r.right-11) return v.kind==0?TTP_SKIN_CLOSE:v.kind==1?TTP_SKIN_PLAYLIST:TTP_SKIN_EQUALIZER;
        if(p.x>=r.right-21) return hitShade;
        if(v.kind==0 && p.x>=244) return TTP_SKIN_MINIMIZE;
        if(v.kind==0 && p.x<16) return TTP_SKIN_MENU;
        return hitDrag;
    }
    if(v.shaded) return 0;
    if(v.kind==0) {
        static constexpr int buttons[]={TTP_SKIN_PREVIOUS,TTP_SKIN_PLAY,TTP_SKIN_PAUSE,TTP_SKIN_STOP,TTP_SKIN_NEXT};
        if(Inside(p,16,88,114,18)) return buttons[(p.x-16)/23];
        if(Inside(p,136,89,22,16)) return TTP_SKIN_OPEN;
        if(Inside(p,164,89,46,15)) return hitShuffle;
        if(Inside(p,210,89,28,15)) return hitRepeat;
        if(Inside(p,219,58,23,12)) return TTP_SKIN_EQUALIZER;
        if(Inside(p,242,58,23,12)) return TTP_SKIN_PLAYLIST;
        if(Inside(p,107,57,68,13)) return hitVolume;
        if(Inside(p,177,57,38,13)) return hitBalance;
        if(Inside(p,16,72,248,10)) return hitSeek;
        if(Inside(p,36,26,64,13)) return TTP_SKIN_TIME_MODE;
    } else if(v.kind==1) {
        if(Inside(p,r.right-18,r.bottom-18,18,18)) return hitResize;
        if(p.y>=20 && p.y<r.bottom-38) {
            if(p.x>=r.right-20) return hitScroll;
            if(p.x>=12) return hitRow+v.scroll+(p.y-20)/13;
        }
        if(p.y>=r.bottom-30 && p.x<40) return TTP_SKIN_OPEN;
        if(p.y>=r.bottom-30 && p.x<75) return TTP_SKIN_REMOVE_ROW;
        if(p.y>=r.bottom-38) return TTP_SKIN_MENU;
    } else {
        if(Inside(p,14,18,25,12)) return TTP_SKIN_EQ_ENABLE;
        if(Inside(p,217,18,44,12)) return TTP_SKIN_EQ_PRESETS;
        for(int i=0;i<11;++i) if(Inside(p,i?78+(i-1)*18:21,38,14,63)) return TTP_SKIN_EQ_VALUE+i;
    }
    return hitDrag;
}
void Skin::Track(View& v,int hit,POINT p) {
    if(hit==hitVolume) Command(TTP_SKIN_VOLUME,std::clamp((int(p.x)-114)*100/51,0,100));
    else if(hit==hitBalance) Command(TTP_SKIN_BALANCE,std::clamp((int(p.x)-196)*100/12,-100,100));
    else if(hit==hitSeek) Command(TTP_SKIN_SEEK,std::clamp((int(p.x)-30)*10000/219,0,10000));
    else if(hit>=TTP_SKIN_EQ_VALUE && hit<TTP_SKIN_EQ_VALUE+11) Command(uint32_t(hit),std::clamp(12-(int(p.y)-44)*24/51,-12,12));
    else if(hit==hitScroll) {
        RECT r{};GetClientRect(v.window,&r);const auto s=State();
        const int max=std::max(0,int(s.track_count)-std::max(1,(int(r.bottom)-58)/13));
        v.scroll=std::clamp((int(p.y)-29)*max/std::max(1,int(r.bottom)-76),0,max);
    }
    InvalidateRect(v.window,nullptr,FALSE);
}
void Skin::Activate(View& v,int hit,POINT p) {
    if(hit==hitShade) ToggleShade(v);
    else if(hit==hitShuffle) Command(TTP_SKIN_MODE,State().mode==4?2:4);
    else if(hit==hitRepeat) Command(TTP_SKIN_MODE,State().mode==1||State().mode==3?2:3);
    else if(hit==TTP_SKIN_REMOVE_ROW) { if(v.selected>=0) Command(TTP_SKIN_REMOVE_ROW,v.selected); }
    else if(hit>0 && hit<hitShade) Command(uint32_t(hit));
    else Track(v,hit,p);
}
LRESULT CALLBACK Skin::Subclass(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR data) {
    auto& v=*reinterpret_cast<View*>(data);
    if(message==WM_NCDESTROY) { v.skin->EndDrag(v);RemoveWindowSubclass(window,Subclass,subclassId);v.window=nullptr;return DefSubclassProc(window,message,wp,lp); }
    try { return v.skin->Message(v,message,wp,lp); }
    catch(...) { return DefSubclassProc(window,message,wp,lp); }
}
bool Skin::HostDrag(View& v,uint32_t phase,POINT point) const {
    if(!host_.drag) return false;
    const TtpSkinDrag event{sizeof(event),phase,v.window,point,
        static_cast<uint32_t>(v.resizing ? TTP_SKIN_DRAG_RIGHT|TTP_SKIN_DRAG_BOTTOM : TTP_SKIN_DRAG_WINDOW),
        {275,116}};
    return host_.drag(host_.context,&event)!=FALSE;
}
void Skin::EndDrag(View& v) {
    // Clear first: the host releases capture synchronously, reentering our
    // WM_CAPTURECHANGED handler before this call returns.
    const bool delegated=std::exchange(v.host_drag,false);
    v.dragging=v.resizing=false;
    if(delegated) HostDrag(v,TTP_SKIN_DRAG_END);
}
LRESULT Skin::Message(View& v,UINT message,WPARAM wp,LPARAM lp) {
    const POINT point{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
    switch(message) {
    case WM_PAINT: { PAINTSTRUCT p{};HDC dc=BeginPaint(v.window,&p);Paint(v.window,dc);EndPaint(v.window,&p);return 0; }
    case WM_PRINTCLIENT: Paint(v.window,reinterpret_cast<HDC>(wp));return 0;
    case WM_ERASEBKGND: return 1;
    case WM_NCHITTEST: return HTCLIENT;
    case WM_SIZE: if(wp!=SIZE_MINIMIZED) {HideChildren(v);Region(v);InvalidateRect(v.window,nullptr,FALSE);} return 0;
    case WM_TIMER:
        if(wp==timerId) { if(v.kind==0) ++ticks_;HideChildren(v);InvalidateRect(v.window,nullptr,FALSE);return 0; } break;
    case WM_ACTIVATE: InvalidateRect(v.window,nullptr,FALSE);break;
    case WM_SETCURSOR: SetCursor(LoadCursorW(nullptr,IDC_ARROW));return TRUE;
    case WM_RBUTTONDOWN: return 0;
    case WM_RBUTTONUP: Command(TTP_SKIN_MENU);return 0;
    case WM_LBUTTONDBLCLK: {
        const int hit=Hit(v,point);
        if(v.kind==1 && hit>=hitRow && uint32_t(hit-hitRow)<State().track_count) Command(TTP_SKIN_PLAY_ROW,hit-hitRow);
        else if(point.y<14) ToggleShade(v);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        SetFocus(v.window);v.pressed=Hit(v,point);
        if(v.pressed>=hitRow) { if(uint32_t(v.pressed-hitRow)<State().track_count) v.selected=v.pressed-hitRow;v.pressed=0;InvalidateRect(v.window,nullptr,FALSE);return 0; }
        if(v.pressed==hitDrag || v.pressed==hitResize) {
            v.dragging=v.pressed==hitDrag;v.resizing=v.pressed==hitResize;
            v.host_drag=HostDrag(v,TTP_SKIN_DRAG_BEGIN,point);
            // The native begin callback already captures. Capturing twice can
            // synchronously cancel this gesture through WM_CAPTURECHANGED.
            if(!v.host_drag) SetCapture(v.window);
            v.drag_start=point;ClientToScreen(v.window,&v.drag_start);GetWindowRect(v.window,&v.drag_rect);
        } else {
            SetCapture(v.window);
            if(v.pressed==hitVolume || v.pressed==hitBalance || v.pressed==hitScroll || (v.pressed>=TTP_SKIN_EQ_VALUE && v.pressed<TTP_SKIN_EQ_VALUE+11)) Track(v,v.pressed,point);
        }
        InvalidateRect(v.window,nullptr,FALSE);return 0;
    }
    case WM_MOUSEMOVE:
        if(GetCapture()==v.window) {
            if(v.dragging || v.resizing) {
                if(v.host_drag) {HostDrag(v,TTP_SKIN_DRAG_MOVE,point);return 0;}
                // Compatibility with older v1 hosts: move this window alone.
                // Full TTPlayer docking requires the optional host callback.
                POINT cursor=point;ClientToScreen(v.window,&cursor);const int dx=cursor.x-v.drag_start.x,dy=cursor.y-v.drag_start.y;
                if(v.resizing) SetWindowPos(v.window,nullptr,0,0,std::clamp(int(v.drag_rect.right-v.drag_rect.left)+dx,275,2000),std::clamp(int(v.drag_rect.bottom-v.drag_rect.top)+dy,116,1600),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
                else SetWindowPos(v.window,nullptr,v.drag_rect.left+dx,v.drag_rect.top+dy,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
            } else if(v.pressed!=hitSeek) Track(v,v.pressed,point);
        }
        return 0;
    case WM_LBUTTONUP: {
        const int hit=v.pressed;v.pressed=0;
        const bool moving=v.dragging||v.resizing;EndDrag(v);
        if(GetCapture()==v.window) ReleaseCapture();
        if(!moving) {
            if(hit==hitSeek || hit==hitVolume || hit==hitBalance || hit==hitScroll || (hit>=TTP_SKIN_EQ_VALUE && hit<TTP_SKIN_EQ_VALUE+11)) Track(v,hit,point);
            else if(hit==Hit(v,point)) Activate(v,hit,point);
        }
        InvalidateRect(v.window,nullptr,FALSE);return 0;
    }
    case WM_CAPTURECHANGED: case WM_CANCELMODE:
        v.pressed=0;EndDrag(v);
        if(message==WM_CANCELMODE && GetCapture()==v.window) ReleaseCapture();
        InvalidateRect(v.window,nullptr,FALSE);return 0;
    case WM_MOUSEWHEEL:
        if(v.kind==1) {v.wheel+=GET_WHEEL_DELTA_WPARAM(wp);v.scroll=std::max(0,v.scroll-(v.wheel/WHEEL_DELTA)*3);v.wheel%=WHEEL_DELTA;InvalidateRect(v.window,nullptr,FALSE);}
        else Command(TTP_SKIN_VOLUME,std::clamp(State().volume+GET_WHEEL_DELTA_WPARAM(wp)/WHEEL_DELTA*5,0,100));
        return 0;
    case WM_KEYDOWN:
        if(v.kind==1 && (wp==VK_RETURN || wp==VK_DELETE)) {if(v.selected>=0) Command(wp==VK_RETURN?TTP_SKIN_PLAY_ROW:TTP_SKIN_REMOVE_ROW,v.selected);return 0;}
        break;
    }
    return DefSubclassProc(v.window,message,wp,lp);
}
}
