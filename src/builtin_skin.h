#pragma once
#include "archive.h"

namespace waskin {
// A reserved virtual package filename; no file is extracted or required.
inline constexpr wchar_t kBuiltinPackage[]=L"@default.wsz";
bool IsBuiltinPackage(const wchar_t* path);
const Archive& BuiltinArchive();
}
