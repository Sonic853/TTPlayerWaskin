#pragma once
#include "ttp_skin_plugin.h"
#include <algorithm>
#include <windowsx.h>

namespace waskin {
inline bool PlaylistDragEnabled(const TtpSkinHost& host) {
    return !host.option || host.option(host.context,TTP_SKIN_PLAYLIST_DRAG_ENABLED)!=0;
}
inline int PlaylistPage(const RECT& bounds,int height) {
    return std::max(1,int(bounds.bottom-bounds.top)/height);
}
inline bool PlaylistDropContains(RECT bounds,POINT point) {
    if(IsRectEmpty(&bounds))return false;
    InflateRect(&bounds,0,2);return PtInRect(&bounds,point)!=FALSE;
}
inline int PlaylistInsertion(const RECT& bounds,int height,POINT point,int& top,int count,bool advance,bool halfRow=false) {
    if(!PlaylistDropContains(bounds,point))return -1;
    const int page=PlaylistPage(bounds,height),maximum=std::max(0,count-page);
    top=std::clamp(top,0,maximum);
    if(advance) {
        if(point.y<bounds.top+height)top=std::max(0,top-1);
        else if(point.y>=bounds.top+(page-1)*height)top=std::min(maximum,top+1);
    }
    return std::clamp(top+std::max(0,int(point.y-bounds.top)+(halfRow?height/2:0))/height,0,count);
}
inline void PlaylistWheel(WPARAM wp,int& remainder,int& top,int count,int page) {
    if(GET_KEYSTATE_WPARAM(wp)&(MK_CONTROL|MK_SHIFT))return;
    UINT lines=3;SystemParametersInfoW(SPI_GETWHEELSCROLLLINES,0,&lines,0);
    if(!lines)return;
    remainder+=GET_WHEEL_DELTA_WPARAM(wp);
    const int steps=remainder/WHEEL_DELTA;remainder%=WHEEL_DELTA;
    // SysListView32 retains one overlapping row for page scrolling, including
    // a numeric lines setting larger than the visible page.
    const int maximumStep=std::max(1,page-1);
    const int amount=lines==WHEEL_PAGESCROLL?maximumStep:int(std::min<UINT>(lines,UINT(maximumStep)));
    top=int(std::clamp<int64_t>(int64_t(top)-int64_t(steps)*amount,0,std::max(0,count-page)));
}
inline POINT PlaylistMenuPoint(const RECT& bounds,int height,int row,int top) {
    return {bounds.left+8,std::clamp<LONG>(bounds.top+std::max(0,row-top)*height+height/2,
        bounds.top,std::max(bounds.top,bounds.bottom-1))};
}
inline void PlaylistContext(const TtpSkinHost& host,HWND window,int row,POINT point,WPARAM keys,bool keyboard=false) {
    ClientToScreen(window,&point);
    const TtpSkinPlaylistContext event{sizeof(event),window,point,row,
        uint32_t(keys)&(MK_CONTROL|MK_SHIFT),keyboard};
    if(host.playlist_context && host.playlist_context(host.context,&event))return;
    if(host.command)host.command(host.context,TTP_SKIN_LIST_MENU,row);
}
}
