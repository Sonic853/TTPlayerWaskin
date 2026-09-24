#pragma once
#include "ttp_maki.h"
#include <stdexcept>
#include <string>
namespace waskin {
// Absolute sibling path, no current-directory or PATH search. Each skin keeps
// its own module reference until all of its script instances are destroyed.
class MakiLibrary {
    HMODULE module_{};
public:
    TtpMakiVM api{sizeof(api)};
    MakiLibrary() {
        HMODULE self{};
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&Available),&self)) throw std::runtime_error("skin module path");
        wchar_t path[32768]{};
        DWORD n=GetModuleFileNameW(self,path,DWORD(std::size(path)));
        if(!n || n>=std::size(path)) throw std::runtime_error("skin module path");
        std::wstring file(path,n);const auto slash=file.find_last_of(L"\\/");
        if(slash==std::wstring::npos) throw std::runtime_error("skin module directory");
        file.resize(slash+1);file+=L"ttp_maki.dll";
        // Missing/corrupt optional VM is a capability fallback, never a modal
        // loader error. Resolve thread-local error mode dynamically for XP.
        using SetThreadMode=BOOL(WINAPI*)(DWORD,LPDWORD);
        const auto threadMode=reinterpret_cast<SetThreadMode>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"SetThreadErrorMode"));
        constexpr DWORD flags=SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX;
        DWORD oldMode{};const bool local=threadMode && threadMode(flags,&oldMode);
        if(local)threadMode(oldMode|flags,nullptr);
        else {oldMode=SetErrorMode(flags);SetErrorMode(oldMode|flags);}
        module_=LoadLibraryW(file.c_str());
        if(local)threadMode(oldMode,nullptr);else SetErrorMode(oldMode);
        auto get=module_?reinterpret_cast<TtpGetMakiVM>(GetProcAddress(module_,TTP_MAKI_ENTRY)):nullptr;
        if(!get || FAILED(get(TTP_MAKI_ABI,&api)) || api.size<TTP_MAKI_VM_V1_SIZE || api.version!=TTP_MAKI_ABI ||
            !api.create || !api.destroy || !api.event) {
            if(module_)FreeLibrary(module_);module_=nullptr;throw std::runtime_error("MAKI VM missing/incompatible");
        }
        if(api.size<sizeof(api))api.create_checked=nullptr;
    }
    ~MakiLibrary(){if(module_)FreeLibrary(module_);}
    MakiLibrary(const MakiLibrary&)=delete;
    static bool Available() noexcept {try{MakiLibrary library;return true;}catch(...){return false;}}
};
}
