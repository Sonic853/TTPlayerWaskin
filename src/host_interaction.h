#pragma once
#include "ttp_skin_plugin.h"
#include <algorithm>
#include <string>

namespace waskin {
inline bool NativeVolume(const TtpSkinHost& host) {
    return host.option && host.option(host.context,TTP_SKIN_VOLUME_DELTA)>=1;
}
inline std::wstring HostStatus(const TtpSkinHost& host) {
    wchar_t text[512]{};
    if(!host.tip || !host.tip(host.context,TTP_SKIN_STATUS_TEXT,0,text,512))return {};
    text[511]=0;return text;
}
inline void VolumeWheel(const TtpSkinHost& host,int delta,int previous) {
    if(!host.command)return;
    // Preserve fractional-wheel semantics and queue relative changes, so a
    // burst of messages cannot repeatedly add to the same stale snapshot.
    if(NativeVolume(host))host.command(host.context,TTP_SKIN_VOLUME_DELTA,delta/40);
    else host.command(host.context,TTP_SKIN_VOLUME,std::clamp(previous+delta/40,0,100));
}
inline void EndVolume(const TtpSkinHost& host) {
    if(NativeVolume(host) && host.command)host.command(host.context,TTP_SKIN_VOLUME_END,0);
}
}
