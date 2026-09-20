#include "skin.h"
#include <new>
#include <memory>
#include <string>
#include <algorithm>
#include <cstring>

namespace {
HRESULT WINAPI Probe(const wchar_t* path,TtpSkinInfo* info) {
    if(!path || !info || info->size<sizeof(*info)) return E_INVALIDARG;
    try {
        waskin::Skin validated(path,nullptr);
        std::wstring name=path;const auto slash=name.find_last_of(L"/\\");if(slash!=std::wstring::npos) name.erase(0,slash+1);
        const auto dot=name.find_last_of(L'.');if(dot!=std::wstring::npos) name.resize(dot);
        wcsncpy_s(info->name,name.c_str(),_TRUNCATE);info->author[0]=0;
        return S_OK;
    } catch(...) {return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);}
}
HRESULT WINAPI Create(const wchar_t* path,const TtpSkinHost* host,void** output) {
    if(!path || !output || (host && (host->size<TTP_SKIN_HOST_V1_SIZE || host->version!=TTP_SKIN_ABI))) return E_INVALIDARG;
    *output=nullptr;
    try {*output=new waskin::Skin(path,host);return S_OK;}
    catch(const std::bad_alloc&) {return E_OUTOFMEMORY;} catch(...) {return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);}
}
HRESULT WINAPI Attach(void* instance,const TtpSkinWindows* windows) {
    if(!instance || !windows || windows->size<TTP_SKIN_WINDOWS_V1_SIZE) return E_INVALIDARG;
    try {return static_cast<waskin::Skin*>(instance)->Attach(*windows);} catch(...) {static_cast<waskin::Skin*>(instance)->Detach();return E_FAIL;}
}
void WINAPI Detach(void* instance) {if(instance) static_cast<waskin::Skin*>(instance)->Detach();}
void WINAPI Destroy(void* instance) {delete static_cast<waskin::Skin*>(instance);}
HBITMAP WINAPI Preview(void* instance) {try {return instance?static_cast<waskin::Skin*>(instance)->Preview():nullptr;} catch(...) {return nullptr;}}
void WINAPI Shade(void* instance) {try {if(instance) static_cast<waskin::Skin*>(instance)->Shade();} catch(...) {}}
void WINAPI Paint(void* instance,HWND window,HDC dc) {try {if(instance && dc) static_cast<waskin::Skin*>(instance)->Paint(window,dc);} catch(...) {}}
BOOL WINAPI Translate(void* instance,const MSG* message) {try {return instance && message && static_cast<waskin::Skin*>(instance)->Translate(*message);} catch(...) {return FALSE;}}
BOOL WINAPI Handles(void* instance,HWND window) {return instance && static_cast<waskin::Skin*>(instance)->Handles(window);}
HMENU WINAPI Menu(void* instance,HWND window,uint32_t command) {try {return instance?static_cast<waskin::Skin*>(instance)->Menu(window,command):nullptr;} catch(...) {return nullptr;}}
BOOL WINAPI ContentState(void* instance,TtpSkinContent* state,BOOL apply) {try {return instance && state && static_cast<waskin::Skin*>(instance)->ContentState(*state,apply!=FALSE);} catch(...) {return FALSE;}}
BOOL WINAPI LyricColors(void* instance,HWND window,TtpSkinLyricColors* colors) {return instance && colors && static_cast<waskin::Skin*>(instance)->LyricColors(window,*colors);}
HRESULT WINAPI Layout(void* instance,TtpSkinLayout* state,BOOL restore) {
    if(!instance || !state || state->size<sizeof(*state)) return E_INVALIDARG;
    try {return static_cast<waskin::Skin*>(instance)->Layout(*state,restore!=FALSE);} catch(...) {return E_FAIL;}
}
}
extern "C" HRESULT WINAPI ttpGetSkinPlugin(uint32_t version,TtpSkinPlugin* output) {
    if(version!=TTP_SKIN_ABI || !output || output->size<TTP_SKIN_PLUGIN_V1_SIZE) return E_INVALIDARG;
    const auto size=static_cast<uint32_t>(std::min<size_t>(output->size,sizeof(*output)));
    const TtpSkinPlugin api{size,TTP_SKIN_ABI,L"Winamp",Probe,Create,Attach,Detach,Destroy,Preview,Shade,Paint,Translate,
        L"waskin",L".wsz;.wal",Layout,Handles,Menu,ContentState,LyricColors};
    std::memcpy(output,&api,size);
    return S_OK;
}
