#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace waskin {
using Bytes = std::vector<uint8_t>;
// In-memory ZIP reader: stored/raw DEFLATE, CRC32, bounded output. No extraction.
class Archive {
public:
    explicit Archive(const wchar_t* path);
    explicit Archive(Bytes bytes);
    bool Has(const std::string& name) const;
    Bytes Read(const std::string& name) const;
private:
    struct Entry { uint32_t offset, compressed, size, crc; uint16_t method, flags; };
    Bytes bytes_;
    std::unordered_map<std::string, Entry> entries_;
    std::string prefix_;
};
}
