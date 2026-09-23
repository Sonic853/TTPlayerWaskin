#include "skin.h"
#include <windowsx.h>

// XP-targeted SDK headers omit this tooltip style; older controls ignore it.
#ifndef TTS_USEVISUALSTYLE
#define TTS_USEVISUALSTYLE 0x100
#endif

namespace waskin {
std::wstring Skin::TipText(const View& view,int hit,const RECT& bounds) const {
    if(!hit || hit==hitDrag) return {};
    if(hit>=hitRow) {
        if(view.kind!=1 || !host_.tip || uint32_t(hit-hitRow)>=State().track_count) return {};
        const auto required=host_.tip(host_.context,TTP_SKIN_TRACK_TIP,hit-hitRow,nullptr,0);
        if(required<=1 || required>1024*1024) return {};
        std::wstring text(size_t(required),L'\0');
        const auto result=host_.tip(host_.context,TTP_SKIN_TRACK_TIP,hit-hitRow,text.data(),uint32_t(text.size()));
        if(result<=1 || result>required) return {};
        text.resize(wcsnlen_s(text.data(),text.size()));return text;
    }
    if(view.kind==3 && hit==TTP_SKIN_LYRICS) return L"关闭歌词／视觉窗口";
    const auto state=State();
    if(bounds.top==3 && view.kind && (hit==TTP_SKIN_PLAYLIST || hit==TTP_SKIN_EQUALIZER))
        return view.kind==1?L"关闭播放列表":L"关闭均衡器";
    uint32_t command=uint32_t(hit);int value=0;
    if(hit>=hitListAdd && hit<=hitListList) {
        static constexpr int toolbar[]={0,1,5,3,2};
        command=TTP_SKIN_LIST_TOOLBAR;value=toolbar[hit-hitListAdd];
    } else if(hit>=TTP_SKIN_EQ_VALUE && hit<TTP_SKIN_EQ_VALUE+11) {
        command=TTP_SKIN_EQ_VALUE;value=hit-TTP_SKIN_EQ_VALUE;
    }
    if(host_.tip) {
        wchar_t label[512]{};
        if(host_.tip(host_.context,command,value,label,uint32_t(std::size(label)))) {
            if(command==TTP_SKIN_EQ_VALUE)
                return (value?L"均衡器频段 "+std::to_wstring(value):std::wstring(L"前置增益"))+L"："+label;
            return label;
        }
    }
    switch(hit) {
    case TTP_SKIN_PLAY: return L"播放";
    case TTP_SKIN_PAUSE: return L"暂停";
    case TTP_SKIN_STOP: return L"停止";
    case TTP_SKIN_PREVIOUS: return L"上一首";
    case TTP_SKIN_NEXT: return L"下一首";
    case TTP_SKIN_OPEN: return L"打开文件";
    case TTP_SKIN_CLOSE: return L"关闭";
    case TTP_SKIN_MINIMIZE: return L"最小化";
    case TTP_SKIN_PLAYLIST: return L"播放列表";
    case TTP_SKIN_EQUALIZER: return L"均衡器";
    case TTP_SKIN_OPTIONS: return L"选项";
    case TTP_SKIN_MENU: return L"主菜单";
    case TTP_SKIN_PROPERTIES: return L"歌曲属性";
    case TTP_SKIN_ALWAYS_ON_TOP: return L"总在最前";
    case TTP_SKIN_EQ_ENABLE: return state.eq_enabled?L"关闭均衡器效果":L"启用均衡器效果";
    case TTP_SKIN_EQ_PRESETS: return L"均衡器预设";
    case TTP_SKIN_TIME_MODE: return state.elapsed?L"切换为剩余时间":L"切换为已播放时间";
    case TTP_SKIN_VISUAL_NEXT: return L"切换可视化效果";
    case TTP_SKIN_VISUAL_MENU: return L"可视化选项";
    case hitVolume: return L"音量："+std::to_wstring(state.volume)+L"%";
    case hitBalance: return state.balance==0?L"声道平衡：居中":std::wstring(L"声道平衡：")+(state.balance<0?L"左 ":L"右 ")+std::to_wstring(state.balance<0?-state.balance:state.balance)+L"%";
    case hitSeek: return state.duration_ms>0?L"播放进度（拖动后松开定位）":L"播放进度（当前不可定位）";
    case hitShuffle: return state.mode==4?L"随机播放（已开启）":L"随机播放（已关闭）";
    case hitRepeat: return state.mode==1||state.mode==3?L"循环播放（已开启）":L"循环播放（已关闭）";
    case hitShade: return view.shaded?L"展开窗口":L"折叠窗口";
    case hitScale: return scale_==1?L"切换为双倍尺寸":L"恢复普通尺寸";
    case hitResize: return L"调整窗口大小";
    case hitScroll: return L"滚动播放列表";
    case hitScrollUp: return L"向上滚动";
    case hitScrollDown: return L"向下滚动";
    case hitAuto: return L"按曲目自动加载均衡器预设（暂不支持）";
    case hitEqUp: return L"全部频段设为 +12 dB";
    case hitEqFlat: return L"全部频段归零";
    case hitEqDown: return L"全部频段设为 -12 dB";
    case hitVideoFullscreen: return L"全屏显示当前内容";
    case hitVideoNormal: return L"恢复普通窗口大小";
    case hitVideoDouble: return L"双倍窗口大小";
    case hitVideoMode: return L"切换歌词／视觉效果／歌词与视觉同屏";
    case hitVideoMenu: return L"显示内容和视觉效果菜单";
    case hitListAdd: return L"添加";
    case hitListRem: return L"删除";
    case hitListSel: return L"编辑／选择";
    case hitListMisc: return L"排序";
    case hitListList: return L"列表";
    default:
        if(hit>=TTP_SKIN_EQ_VALUE && hit<TTP_SKIN_EQ_VALUE+11)
            return (hit==TTP_SKIN_EQ_VALUE?std::wstring(L"前置增益"):L"均衡器频段 "+std::to_wstring(hit-TTP_SKIN_EQ_VALUE))+L"："+std::to_wstring(state.eq[hit-TTP_SKIN_EQ_VALUE])+L" dB"+(state.eq_enabled?L"":L"（效果已关闭）");
        return {};
    }
}
void Skin::HideTip(View& view) noexcept {
    if(IsWindow(view.tooltip)) {
        SendMessageW(view.tooltip,TTM_POP,0,0);
        if(view.tip_hit) {
            TOOLINFOW tool{TTTOOLINFOW_V2_SIZE};tool.hwnd=view.window;tool.uId=UINT_PTR(view.tip_hit);
            SendMessageW(view.tooltip,TTM_DELTOOLW,0,reinterpret_cast<LPARAM>(&tool));
        }
    }
    view.tip_hit=0;view.tip_text.clear();
}
void Skin::UpdateTip(View& view,POINT point) {
    for(auto& other:views_) if(&other!=&view && other.tip_hit) HideTip(other);
    if(GetCapture() || !IsWindowEnabled(view.window)) {HideTip(view);return;}
    RECT bounds{};const int hit=Hit(view,point,&bounds);
    // Defer row metadata queries until the tooltip's normal hover delay has
    // elapsed. Merely moving over a song must not format/read its tags.
    if(hit>=hitRow ? view.kind!=1 || !host_.tip || uint32_t(hit-hitRow)>=State().track_count
                   : TipText(view,hit,bounds).empty()) {HideTip(view);return;}
    if(!view.tooltip) {
        view.tooltip=CreateWindowExW(WS_EX_TOPMOST,TOOLTIPS_CLASSW,nullptr,TTS_NOPREFIX | TTS_USEVISUALSTYLE,
            CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,view.window,nullptr,
            reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(view.window,GWLP_HINSTANCE)),nullptr);
        if(!view.tooltip) return;
        SendMessageW(view.tooltip,CCM_SETUNICODEFORMAT,TRUE,0);
        SendMessageW(view.tooltip,TTM_SETMAXTIPWIDTH,0,400);
    }
    const int scale=(view.kind==1 || view.kind==3)?1:scale_;
    if(view.tip_hit!=hit || !EqualRect(&view.tip_bounds,&bounds)) {
        HideTip(view);view.tip_hit=hit;view.tip_bounds=bounds;
        // v5 common controls reject the v6 reserved tail. No v6 fields are used.
        TOOLINFOW tool{TTTOOLINFOW_V2_SIZE};tool.hwnd=view.window;tool.uId=UINT_PTR(hit);
        tool.rect={bounds.left*scale,bounds.top*scale,bounds.right*scale,bounds.bottom*scale};
        tool.lpszText=LPSTR_TEXTCALLBACKW;
        if(!SendMessageW(view.tooltip,TTM_ADDTOOLW,0,reinterpret_cast<LPARAM>(&tool))) {view.tip_hit=0;return;}
    }
    TRACKMOUSEEVENT leave{sizeof(leave),TME_LEAVE,view.window,0};TrackMouseEvent(&leave);
    MSG event{};event.hwnd=view.window;event.message=WM_MOUSEMOVE;
    event.lParam=MAKELPARAM(point.x*scale,point.y*scale);event.time=GetMessageTime();
    event.pt={point.x*scale,point.y*scale};ClientToScreen(view.window,&event.pt);
    SendMessageW(view.tooltip,TTM_RELAYEVENT,0,reinterpret_cast<LPARAM>(&event));
}
void Skin::RefreshRowTip(View& view) {
    if(view.kind!=1 || view.tip_hit<hitRow || !view.tooltip) return;
    POINT point{};RECT bounds{};
    if(GetCapture() || !GetCursorPos(&point) || !ScreenToClient(view.window,&point) ||
       Hit(view,point,&bounds)!=view.tip_hit || !EqualRect(&bounds,&view.tip_bounds) ||
       uint32_t(view.tip_hit-hitRow)>=State().track_count) {HideTip(view);return;}
    // Metadata arrives asynchronously. Refresh only a visible song tip, at
    // most twice a second, without restarting its hover/autopop timers.
    const DWORD now=GetTickCount();
    if(!IsWindowVisible(view.tooltip) || now-view.tip_tick<500) return;
    view.tip_tick=now;
    const auto text=TipText(view,view.tip_hit,view.tip_bounds);
    if(text.empty()) {HideTip(view);return;}
    if(text!=view.tip_text) SendMessageW(view.tooltip,TTM_UPDATE,0,0);
}
}
