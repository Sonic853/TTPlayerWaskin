#include "skin.h"
#include <algorithm>
#include <cstring>

namespace waskin {
// Last-resort artwork only for resources absent from the embedded base skin.
Image MakeFallback(const char* name,int width,int height) {
    Image out;out.width=width;out.height=height;
    HDC screen=GetDC(nullptr),dc=CreateCompatibleDC(screen);
    out.bitmap=CreateCompatibleBitmap(screen,width,height);ReleaseDC(nullptr,screen);
    if(!dc || !out.bitmap) {if(dc) DeleteDC(dc);return out;}
    const auto old=SelectObject(dc,out.bitmap);
    const auto fill=[&](int x,int y,int w,int h,COLORREF c) {
        RECT r{x,y,x+w,y+h};SetDCBrushColor(dc,c);FillRect(dc,&r,static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
    };
    HFONT font=CreateFontW(-9,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"Tahoma");
    const auto previous=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(220,240,220));
    const auto label=[&](int x,int y,int w,int h,const wchar_t* t) {
        RECT r{x,y,x+w,y+h};DrawTextW(dc,t,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
    };
    const auto button=[&](int x,int y,int w,int h,const wchar_t* t,bool down=false) {
        fill(x,y,w,h,RGB(65,73,79));RECT r{x,y,x+w,y+h};DrawEdge(dc,&r,down?EDGE_SUNKEN:EDGE_RAISED,BF_RECT);label(x,y,w,h,t);
    };
    fill(0,0,width,height,RGB(40,48,55));
    const std::string n=name;
    if(n=="main.bmp") {
        fill(24,24,76,39,RGB(0,0,0));fill(111,24,154,23,RGB(0,0,0));
        label(126,43,30,7,L"kbps");label(170,43,25,7,L"kHz");
    } else if(n=="cbuttons.bmp") {
        const wchar_t* labels[]={L"|<",L">",L"||",L"[]",L">|"};
        for(int j=0;j<2;++j) for(int i=0;i<5;++i) button(i*23,j*18,i==4?22:23,18,labels[i],j!=0);
        for(int j=0;j<2;++j) button(114,j*16,22,16,L"+",j!=0);
    } else if(n=="titlebar.bmp") {
        for(int y:{0,15,29,42}) {fill(27,y,275,14,RGB(48,67,85));label(75,y,130,13,y<29?L"TTPlayer":L"TT");}
        for(int j=0;j<2;++j) {button(0,j*9,9,9,L"=",j!=0);button(9,j*9,9,9,L"-",j!=0);button(18,j*9,9,9,L"x",j!=0);button(j*9,18,9,9,L"_",j!=0);button(j*9,27,9,9,L"+",j!=0);}
        label(304,0,8,43,L"O A I D V");
    } else if(n=="shufrep.bmp") {
        for(int j=0;j<4;++j) {button(0,j*15,28,15,j>=2?L"REP*":L"REP",j%2);button(28,j*15,47,15,j>=2?L"SHUF*":L"SHUF",j%2);}
        for(int j=0;j<2;++j) for(int i=0;i<4;++i) button(i*23,61+j*12,23,12,i%2?L"PL":L"EQ",i>=2);
    } else if(n=="volume.bmp" || n=="balance.bmp") {
        for(int i=0;i<28;++i) {fill(0,i*15,68,13,RGB(20,25,28));fill(3,i*15+5,2+i*2,3,RGB(70,160,80));}
        button(0,422,14,11,L"",true);button(15,422,14,11,L"",false);
    } else if(n=="posbar.bmp") {
        fill(0,0,248,10,RGB(12,17,20));button(248,0,29,10,L"",false);button(278,0,29,10,L"",true);
    } else if(n=="text.bmp") {
        fill(0,0,width,height,RGB(0,0,0));SetTextColor(dc,RGB(70,240,95));
        HFONT tiny=CreateFontW(-6,5,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,FIXED_PITCH,L"Small Fonts");
        SelectObject(dc,tiny);
        const wchar_t* rows[]={L"ABCDEFGHIJKLMNOPQRSTUVWXYZ",L"0123456789\1.:()-'!_+\\/[]^&%,=$#",L"   ?*"};
        for(int row=0;row<3;++row) for(int i=0;rows[row][i];++i) {
            RECT r{i*5,row*6,i*5+5,row*6+6};ExtTextOutW(dc,i*5,row*6,ETO_CLIPPED,&r,rows[row]+i,1,nullptr);
        }
        SelectObject(dc,font);DeleteObject(tiny);
    } else if(n=="numbers.bmp" || n=="nums_ex.bmp") {
        fill(0,0,width,height,RGB(0,0,0));SetTextColor(dc,RGB(70,240,95));
        for(int i=0;i<10;++i) {wchar_t t[]{wchar_t(L'0'+i),0};label(i*9,0,9,13,t);}label(99,0,9,13,L"-");
    } else if(n=="playpaus.bmp") {
        fill(0,0,width,height,RGB(0,0,0));label(0,0,9,9,L">");label(9,0,9,9,L"||");label(18,0,9,9,L"[]");
    } else if(n=="monoster.bmp") {
        for(int j=0;j<2;++j) {SetTextColor(dc,j?RGB(100,110,105):RGB(100,255,140));label(0,j*12,29,12,L"stereo");label(29,j*12,28,12,L"mono");}
    } else if(n=="eqmain.bmp") {
        for(int y:{134,149}) label(0,y,275,14,L"TTPlayer Equalizer");
        for(int j=0;j<4;++j) {button(10+j*59,119,25,12,j%2?L"ON*":L"ON",j>=2);button(35+j*59,119,33,12,L"AUTO",false);}
        button(224,164,44,12,L"Presets");button(224,176,44,12,L"Presets",true);
        for(int i=0;i<28;++i) {int x=13+(i%14)*15,y=i<14?164:229;fill(x,y,14,63,RGB(15,20,22));fill(x+6,y+3,2,57,RGB(75,100,80));}
        button(0,164,11,11,L"");button(0,176,11,11,L"",true);
        fill(0,294,113,19,RGB(0,0,0));fill(0,314,113,1,RGB(100,100,100));fill(115,294,1,19,RGB(60,255,90));
        label(40,31,28,14,L"+12");label(40,63,28,14,L"0");label(40,91,28,14,L"-12");
        button(0,116,9,9,L"x");button(0,125,9,9,L"x",true);button(254,137,9,9,L"_");button(264,137,9,9,L"x");
    } else if(n=="eq_ex.bmp") {
        for(int y:{0,15}) {label(0,y,60,14,L"EQ");fill(61,y+3,102,7,RGB(10,15,20));fill(166,y+3,44,7,RGB(10,15,20));button(254,y+3,9,9,L"+");button(264,y+3,9,9,L"x");}
        for(int i=0;i<6;++i) fill(1+i*3,30,3,7,RGB(130,190,150));
        button(1,38,9,9,L"_",true);button(1,47,9,9,L"+",true);button(11,38,9,9,L"x");button(11,47,9,9,L"x",true);
    } else if(n=="video.bmp") {
        for(int y:{0,21}) {
            label(26,y,100,20,L"TTPlayer Lyrics");
            button(167,y+3,9,9,L"x");
        }
        button(148,42,9,9,L"x",true);
        const wchar_t* icons[]={L"F",L"1",L"2",L"T",L"..."};
        for(int i=0;i<5;++i) {
            button(9+i*15,51,15,18,icons[i]);
            button(158+i*15,42,15,18,icons[i],true);
        }
    } else if(n=="pledit.bmp") {
        label(26,0,100,20,L"TTPlayer Playlist");label(26,21,100,20,L"TTPlayer Playlist");
        // Toolbar artwork lives in the two bottom strips, including LIST in
        // the right-hand strip. These project-owned icons are used only when
        // the skin does not supply that strip; never cover valid skin pixels.
        const int xs[]={14,43,72,101,232};
        const COLORREF ink=RGB(220,240,220);
        // The fixed playlist time slots leave the colon in pledit.bmp.
        fill(210,96,1,1,ink);fill(210,99,1,1,ink);
        for(int i=0;i<5;++i) {
            const int x=xs[i];button(x,80,22,18,L"");
            if(i<2) {fill(x+6,88,10,2,ink);if(i==0) fill(x+10,84,2,10,ink);}
            else if(i==2) {
                for(int step=0;step<4;++step) fill(x+5+step,88+step,2,2,ink);
                for(int step=0;step<7;++step) fill(x+9+step,91-step,2,2,ink);
            } else if(i==3) {
                for(int y:{85,90}) for(int dx:{6,11}) fill(x+dx,y,4,4,ink);
            } else for(int y:{84,88,92}) fill(x+6,y,10,2,ink);
        }
        button(52,53,8,18,L"");button(61,53,8,18,L"",true);button(52,42,9,9,L"x",true);button(62,42,9,9,L"_",true);
        button(158,3,9,9,L"_");button(167,3,9,9,L"x");button(128,45,9,9,L"+");button(150,42,9,9,L"+",true);
    }
    SelectObject(dc,previous);DeleteObject(font);SelectObject(dc,old);DeleteDC(dc);return out;
}

HCURSOR ReadCursor(const Bytes& bytes) {
    // ICO/CUR directory entries are packed 16-byte records. CUR image payloads
    // use ICON DIBs; RT_CURSOR prepends the hotspot before that payload.
    const auto u16=[&](size_t at) {uint16_t n{};memcpy(&n,bytes.data()+at,2);return n;};
    const auto u32=[&](size_t at) {uint32_t n{};memcpy(&n,bytes.data()+at,4);return n;};
    if(bytes.size()<22 || u16(0)!=0 || u16(2)!=2 || !u16(4) || u16(4)>128 || 6u+size_t(u16(4))*16>bytes.size()) return nullptr;
    for(unsigned i=0;i<u16(4);++i) {
        size_t at=6+i*16,offset=u32(at+12),length=u32(at+8);
        const unsigned width=bytes[at]?bytes[at]:256,height=bytes[at+1]?bytes[at+1]:256;
        if(width>64 || height>64 || u16(at+4)>=width || u16(at+6)>=height || offset>bytes.size() || length>bytes.size()-offset || length<40) continue;
        BITMAPINFOHEADER info{};memcpy(&info,bytes.data()+offset,sizeof(info));
        if(info.biSize!=40 || info.biWidth!=int(width) || info.biHeight!=int(height)*2 || info.biPlanes!=1 || info.biCompression!=BI_RGB ||
           (info.biBitCount!=1 && info.biBitCount!=4 && info.biBitCount!=8 && info.biBitCount!=24 && info.biBitCount!=32)) continue;
        const size_t palette=info.biBitCount<=8?(info.biClrUsed?info.biClrUsed:1u<<info.biBitCount):0;
        if(palette>256 || 40+palette*4+((width*info.biBitCount+31)/32*4+(width+31)/32*4)*height>length) continue;
        Bytes data(length+4);memcpy(data.data(),bytes.data()+at+4,4);memcpy(data.data()+4,bytes.data()+offset,length);
        if(auto cursor=reinterpret_cast<HCURSOR>(CreateIconFromResourceEx(data.data(),DWORD(data.size()),FALSE,0x30000,width,height,LR_DEFAULTCOLOR))) return cursor;
    }
    return nullptr;
}
}
