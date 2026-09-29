#pragma once
#include "ttp_skin_plugin.h"
#include <algorithm>
#include <cstring>

namespace waskin {
// Only playlist rows use this cache. Skin titles/chrome and saved lyric fonts
// retain their own descriptors, even when Options changes the row font live.
class PlaylistFontCache {
    HFONT font_{};
    LOGFONTW descriptor_{};
    int height_{14};
public:
    PlaylistFontCache()=default;
    PlaylistFontCache(const PlaylistFontCache&)=delete;
    PlaylistFontCache& operator=(const PlaylistFontCache&)=delete;
    ~PlaylistFontCache(){if(font_)DeleteObject(font_);}
    HFONT Font()const{return font_;}
    const LOGFONTW& Descriptor()const{return descriptor_;}
    int Height()const{return height_;}
    bool Update(const TtpSkinHost& host,const LOGFONTW& fallback,bool live) {
        LOGFONTW next=fallback;
        if(live && host.playlist_font) {
            LOGFONTW chosen{};
            if(host.playlist_font(host.context,&chosen) && chosen.lfHeight &&
               chosen.lfHeight>=-4096 && chosen.lfHeight<=4096)next=chosen;
        }
        next.lfFaceName[LF_FACESIZE-1]=0;
        if(font_ && !std::memcmp(&descriptor_,&next,sizeof(next)))return false;
        HFONT created=CreateFontIndirectW(&next);if(!created)return false;
        HDC dc=CreateCompatibleDC(nullptr);
        if(!dc){DeleteObject(created);return false;}
        const auto old=SelectObject(dc,created);TEXTMETRICW metrics{};
        const bool measured=GetTextMetricsW(dc,&metrics)!=FALSE;
        SelectObject(dc,old);DeleteDC(dc);
        if(!measured){DeleteObject(created);return false;}
        if(font_)DeleteObject(font_);
        font_=created;descriptor_=next;height_=std::max<LONG>(1,metrics.tmHeight);
        return true;
    }
};
}
