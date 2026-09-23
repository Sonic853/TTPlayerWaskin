#include "builtin_skin.h"
#include <windows.h>
#include <cwchar>
#include <stdexcept>

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace waskin {
bool IsBuiltinPackage(const wchar_t* path) {
    if(!path) return false;
    const wchar_t* name=path;
    for(const wchar_t* p=path;*p;++p) if(*p==L'/' || *p==L'\\') name=p+1;
    return _wcsicmp(name,kBuiltinPackage)==0;
}
const Archive& BuiltinArchive() {
    static const Archive archive([] {
        const auto module=reinterpret_cast<HMODULE>(&__ImageBase);
        const auto resource=FindResourceW(module,MAKEINTRESOURCEW(101),RT_RCDATA);
        const auto data=resource?LoadResource(module,resource):nullptr;
        const auto bytes=data?static_cast<const uint8_t*>(LockResource(data)):nullptr;
        const DWORD size=resource?SizeofResource(module,resource):0;
        if(!bytes || !size) throw std::runtime_error("Missing built-in classic skin");
        return Bytes(bytes,bytes+size);
    }());
    return archive;
}
}
