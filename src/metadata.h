#pragma once
#include "archive.h"

namespace waskin {
struct Metadata {
    std::wstring name,author,email,website;
};
// Optional classic skininfo.xml. Missing/invalid metadata leaves fields empty.
Metadata ReadMetadata(const Bytes& bytes);
}
