#include "skin.h"
#include <algorithm>

namespace waskin {
namespace {
constexpr UINT modeFirst=700, effectFirst=710, actionFirst=720;
const wchar_t* const modes[]={L"歌词",L"视觉效果",L"歌词与视觉同屏"};
const wchar_t* const effects[]={L"无",L"梦幻",L"频谱分析",L"波形",L"专辑封面"};
}

void Skin::DrawVideo(View& view,HDC dc,int width,int height) {
    // Winamp draw_vw.cpp: 25px ends, a 100px caption, 25px repeating
    // title pieces, 11/8px sides and a 38px bottom strip. Partial pieces
    // stretch exactly as the upstream renderer does at arbitrary sizes.
    const int title_y=GetForegroundWindow()==view.window?0:21;
    int x=25;
    const int pieces=std::max(0,width-150)/25;
    Blit(dc,"video.bmp",0,0,25,20,0,title_y);
    if(pieces&1) {Blit(dc,"video.bmp",x,0,12,20,127,title_y);x+=12;}
    for(int i=0;i<pieces/2;++i,x+=25) Blit(dc,"video.bmp",x,0,25,20,127,title_y);
    Blit(dc,"video.bmp",x,0,100,20,26,title_y);x+=100;
    if(pieces&1) {Blit(dc,"video.bmp",x,0,13,20,127,title_y);x+=13;}
    for(int i=0;i<pieces/2;++i,x+=25) Blit(dc,"video.bmp",x,0,25,20,127,title_y);
    if(x<width-25) Blit(dc,"video.bmp",x,0,width-25-x,20,127,title_y,25,20);
    Blit(dc,"video.bmp",width-25,0,25,20,153,title_y);
    if(view.pressed==TTP_SKIN_LYRICS && view.hot)
        Blit(dc,"video.bmp",width-11,3,9,9,148,42);
    for(int y=20;y<height-38;y+=29) {
        const int h=std::min(29,height-38-y);
        Blit(dc,"video.bmp",0,y,11,h,127,42,11,29);
        Blit(dc,"video.bmp",width-8,y,8,h,139,42,8,29);
    }
    Blit(dc,"video.bmp",0,height-38,125,38,0,42);
    for(x=125;x<width-125;x+=25)
        Blit(dc,"video.bmp",x,height-38,std::min(25,width-125-x),38,127,81,25,38);
    Blit(dc,"video.bmp",width-125,height-38,125,38,0,81);
    for(int i=0;i<5;++i) {
        const bool pressed=view.pressed==hitVideoFullscreen+i && view.hot;
        Blit(dc,"video.bmp",9+i*15,height-29,15,18,pressed?158+i*15:9+i*15,pressed?42:51);
    }
    RECT info{92,height-27,width-25,height-13};
    const auto old_font=SelectObject(dc,font_?font_:GetStockObject(DEFAULT_GUI_FONT));
    SetTextColor(dc,video_text_);SetBkColor(dc,video_background_);SetBkMode(dc,OPAQUE);
    DrawTextW(dc,modes[layout_.content_mode-1],-1,&info,DT_SINGLELINE|DT_LEFT|DT_NOPREFIX);
    SelectObject(dc,old_font);
    RECT content{11,20,width-8,height-38};
    if(host_.content && !IsRectEmpty(&content)) {
        const int saved=SaveDC(dc);
        IntersectClipRect(dc,content.left,content.top,content.right,content.bottom);
        host_.content(host_.context,dc,&content,layout_.content_mode,layout_.visual_type);
        if(saved) RestoreDC(dc,saved);
    }
}

void Skin::VideoAction(View& view,int hit) {
    if(hit==hitVideoFullscreen)
        Command(TTP_SKIN_CONTENT_FULLSCREEN,layout_.content_mode|(layout_.visual_type<<8));
    else if(hit==hitVideoNormal || hit==hitVideoDouble) {
        // Scale the content area, keeping the skinned frame and buttons at
        // their original pixel size (Winamp's 100%/200% video buttons).
        const int factor=hit==hitVideoDouble?2:1;
        const SIZE size{256*factor+19,232*factor+58};
        if(!(host_.resize && host_.resize(host_.context,view.window,size)))
            SetWindowPos(view.window,nullptr,0,0,size.cx,size.cy,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    } else if(hit==hitVideoMode) layout_.content_mode=layout_.content_mode%3+1;
    else if(hit==hitVideoMenu) Command(TTP_SKIN_CONTENT_MENU);
    VideoContentChanged();
}

bool Skin::ContentState(TtpSkinContent& state,bool apply) {
    const HWND window=views_[3].window;
    if(state.size<sizeof(state) || !window || state.window!=window) return false;
    if(apply) {
        if(state.mode<1 || state.mode>3 || state.visual_type>4) return false;
        // A mode change must never commit a partially dragged lyric seek.
        SendMessageW(window,WM_CANCELMODE,0,0);
        layout_.content_mode=state.mode;layout_.visual_type=state.visual_type;
        VideoContentChanged();
    }
    RECT client{};GetClientRect(window,&client);
    state.bounds={11,20,client.right-8,client.bottom-38};
    state.mode=layout_.content_mode;state.visual_type=layout_.visual_type;
    return true;
}

bool Skin::LyricColors(HWND window,TtpSkinLyricColors& colors) const {
    // A null HWND queries package defaults before attach, so the host can
    // overlay the saved per-skin/user colours before creating any controls.
    if(colors.size<sizeof(colors) || (window && window!=views_[3].window)) return false;
    colors.text=normal_;colors.highlight=current_;colors.background=background_;
    return true;
}

bool Skin::ContentMinimum(HWND window,SIZE& size) const {
    if(!window || window!=views_[3].window) return false;
    size={275,116};return true;
}

void Skin::VideoContentChanged() {
    CaptureLayout();
    auto& view=views_[3];
    if(host_.content_input && view.window) {
        TtpSkinContent content{sizeof(content),view.window};ContentState(content,false);
        const MSG event{view.window,WM_SIZE};LRESULT result{};
        host_.content_input(host_.context,&content,&event,&result);
    }
    InvalidateRect(view.window,nullptr,FALSE);
}

HMENU Skin::Menu(HWND window,uint32_t command) {
    auto& view=views_[3];
    if(!window || window!=view.window) return nullptr;
    if(command) {
        SendMessageW(window,WM_CANCELMODE,0,0);
        if(command>=modeFirst && command<modeFirst+3) layout_.content_mode=int(command-modeFirst)+1;
        else if(command>=effectFirst && command<effectFirst+5) layout_.visual_type=int(command-effectFirst);
        else if(command>=actionFirst && command<actionFirst+5) VideoAction(view,hitVideoFullscreen+int(command-actionFirst));
        VideoContentChanged();return nullptr;
    }
    HMENU root=CreatePopupMenu(),visual=CreatePopupMenu();
    if(!root || !visual) {if(root) DestroyMenu(root);if(visual) DestroyMenu(visual);return nullptr;}
    for(UINT i=0;i<3;++i) AppendMenuW(root,MF_STRING|(layout_.content_mode==int(i)+1?MF_CHECKED:0),modeFirst+i,modes[i]);
    AppendMenuW(root,MF_SEPARATOR,0,nullptr);
    for(UINT i=0;i<5;++i) AppendMenuW(visual,MF_STRING|(layout_.visual_type==int(i)?MF_CHECKED:0),effectFirst+i,effects[i]);
    AppendMenuW(root,MF_POPUP,reinterpret_cast<UINT_PTR>(visual),L"视觉效果类型");
    AppendMenuW(root,MF_STRING,actionFirst,L"全屏显示当前内容");
    AppendMenuW(root,MF_STRING,actionFirst+1,L"普通窗口大小");
    AppendMenuW(root,MF_STRING,actionFirst+2,L"双倍窗口大小");
    return root;
}
}
