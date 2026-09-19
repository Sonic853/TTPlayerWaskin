#include "archive.h"
#include <windows.h>
#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>

namespace waskin {
namespace {
[[noreturn]] void Invalid() { throw std::runtime_error("Invalid or unsupported WSZ archive"); }
uint16_t U16(const uint8_t* p) { return uint16_t(p[0] | (unsigned(p[1]) << 8)); }
uint32_t U32(const uint8_t* p) { return uint32_t(U16(p)) | (uint32_t(U16(p+2)) << 16); }
void Range(size_t total, size_t start, size_t length) {
    if (start > total || length > total-start) Invalid();
}
std::string Name(std::string value) {
    for (char& c : value) {
        if (c == '\\') c = '/';
        if (c >= 'A' && c <= 'Z') c = char(c + ('a'-'A'));
    }
    if (value.empty() || value[0]=='/' || value.find(':')!=std::string::npos ||
        value.find('\0')!=std::string::npos) Invalid();
    size_t begin=0;
    while (begin < value.size()) {
        auto end=value.find('/',begin);
        const auto part=value.substr(begin,end==std::string::npos ? end : end-begin);
        if (part==".." || part==".") Invalid();
        if (end==std::string::npos) break;
        begin=end+1;
    }
    return value;
}
struct Bits {
    const uint8_t* data; size_t size, bit{};
    unsigned Get(unsigned count) {
        if (count > 16 || bit > size*8 || count > size*8-bit) Invalid();
        unsigned value=0;
        for (unsigned i=0;i<count;++i,++bit) value |= unsigned((data[bit/8]>>(bit%8))&1)<<i;
        return value;
    }
};
struct Huffman {
    struct Node { int child[2]{-1,-1}; int symbol{-1}; };
    std::vector<Node> nodes{1};
    explicit Huffman(const std::vector<unsigned>& lengths) {
        std::array<unsigned,16> counts{}, next{};
        for (auto n:lengths) { if(n>15) Invalid(); if(n) ++counts[n]; }
        int available=1;
        for(unsigned n=1;n<=15;++n) { available=available*2-int(counts[n]); if(available<0) Invalid(); }
        unsigned code=0;
        for(unsigned n=1;n<=15;++n) { code=(code+counts[n-1])<<1; next[n]=code; }
        for(size_t symbol=0;symbol<lengths.size();++symbol) {
            const auto length=lengths[symbol]; if(!length) continue;
            code=next[length]++; int node=0;
            for(unsigned n=length;n;--n) {
                const unsigned direction=(code>>(n-1))&1;
                if(nodes[node].symbol>=0) Invalid();
                int child=nodes[node].child[direction];
                if(child<0) { child=int(nodes.size()); nodes[node].child[direction]=child; nodes.emplace_back(); }
                node=child;
            }
            if(nodes[node].symbol>=0) Invalid();
            nodes[node].symbol=int(symbol);
        }
    }
    unsigned Decode(Bits& bits) const {
        int node=0;
        for(unsigned n=0;n<16;++n) {
            if(nodes[node].symbol>=0) return unsigned(nodes[node].symbol);
            node=nodes[node].child[bits.Get(1)]; if(node<0) Invalid();
        }
        Invalid();
    }
};
Bytes Inflate(const uint8_t* source,size_t size,size_t expected) {
    static constexpr unsigned lengthBase[]={3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
    static constexpr unsigned lengthBits[]={0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
    static constexpr unsigned distanceBase[]={1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
    static constexpr unsigned distanceBits[]={0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
    Bits bits{source,size}; Bytes result; result.reserve(expected);
    bool final=false;
    while(!final) {
        final=bits.Get(1)!=0; const auto type=bits.Get(2);
        if(type==0) {
            bits.bit=(bits.bit+7)&~size_t(7);
            const auto length=bits.Get(16), inverse=bits.Get(16);
            if((length^inverse)!=65535 || length>expected-result.size()) Invalid();
            for(unsigned n=0;n<length;++n) result.push_back(uint8_t(bits.Get(8)));
            continue;
        }
        std::vector<unsigned> literals(288), distances(32,5);
        if(type==1) {
            for(unsigned i=0;i<288;++i) literals[i]=i<144?8:i<256?9:i<280?7:8;
        } else if(type==2) {
            const unsigned lc=bits.Get(5)+257, dc=bits.Get(5)+1, cc=bits.Get(4)+4;
            if(lc>286) Invalid();
            static constexpr unsigned order[]={16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
            std::vector<unsigned> codeLengths(19);
            for(unsigned i=0;i<cc;++i) codeLengths[order[i]]=bits.Get(3);
            Huffman codes(codeLengths); std::vector<unsigned> lengths;
            while(lengths.size()<lc+dc) {
                const auto code=codes.Decode(bits);
                if(code<16) lengths.push_back(code);
                else {
                    if(code>18 || (code==16 && lengths.empty())) Invalid();
                    const auto value=code==16?lengths.back():0;
                    const unsigned repeat=code==16?bits.Get(2)+3:code==17?bits.Get(3)+3:bits.Get(7)+11;
                    if(repeat>lc+dc-lengths.size()) Invalid();
                    lengths.insert(lengths.end(),repeat,value);
                }
            }
            literals.assign(lengths.begin(),lengths.begin()+lc);
            distances.assign(lengths.begin()+lc,lengths.end());
        } else Invalid();
        if(!literals[256]) Invalid();
        Huffman literalTree(literals), distanceTree(distances);
        for(;;) {
            const auto symbol=literalTree.Decode(bits);
            if(symbol==256) break;
            if(symbol<256) { if(result.size()==expected) Invalid(); result.push_back(uint8_t(symbol)); }
            else {
                if(symbol>285) Invalid();
                const auto index=symbol-257;
                const auto length=lengthBase[index]+bits.Get(lengthBits[index]);
                const auto distanceSymbol=distanceTree.Decode(bits);
                if(distanceSymbol>=30) Invalid();
                const auto distance=distanceBase[distanceSymbol]+bits.Get(distanceBits[distanceSymbol]);
                if(distance>result.size() || length>expected-result.size()) Invalid();
                for(unsigned n=0;n<length;++n) result.push_back(result[result.size()-distance]);
            }
        }
    }
    if(result.size()!=expected || (bits.bit+7)/8!=size) Invalid();
    return result;
}
uint32_t Crc(const Bytes& bytes) {
    uint32_t value=0xffffffff;
    for(auto byte:bytes) { value^=byte; for(int n=0;n<8;++n) value=(value>>1)^(0xedb88320u & (0u-(value&1))); }
    return ~value;
}
}

Archive::Archive(const wchar_t* path) {
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE) Invalid();
    LARGE_INTEGER size{}; DWORD read{};
    if(!GetFileSizeEx(file,&size) || size.QuadPart<22 || size.QuadPart>64*1024*1024) { CloseHandle(file); Invalid(); }
    try { bytes_.resize(size_t(size.QuadPart)); } catch(...) { CloseHandle(file); throw; }
    const BOOL ok=ReadFile(file,bytes_.data(),DWORD(bytes_.size()),&read,nullptr); CloseHandle(file);
    if(!ok || read!=bytes_.size()) Invalid();
    size_t end=bytes_.size()-22; const size_t lower=end>65535?end-65535:0;
    for(;;) {
        if(U32(bytes_.data()+end)==0x06054b50 && end+22+U16(bytes_.data()+end+20)==bytes_.size()) break;
        if(end==lower) Invalid(); --end;
    }
    const auto* e=bytes_.data()+end;
    if(U16(e+4)||U16(e+6)||U16(e+8)!=U16(e+10)) Invalid();
    const auto count=U16(e+10); if(count>4096) Invalid();
    size_t offset=U32(e+16), centralEnd=offset+size_t(U32(e+12));
    if(centralEnd!=end) Invalid();
    uint64_t total=0;
    for(unsigned i=0;i<count;++i) {
        Range(end,offset,46); const auto* c=bytes_.data()+offset;
        if(U32(c)!=0x02014b50 || U16(c+34)) Invalid();
        const size_t nameLength=U16(c+28), length=46+nameLength+U16(c+30)+U16(c+32);
        Range(end,offset,length);
        auto name=Name(std::string(reinterpret_cast<const char*>(c+46),nameLength));
        Entry entry{U32(c+42),U32(c+20),U32(c+24),U32(c+16),U16(c+10),U16(c+8)};
        if((entry.flags & 0x2041) || (entry.method!=0 && entry.method!=8) || entry.size>16*1024*1024) Invalid();
        total+=entry.size; if(total>128*1024*1024) Invalid();
        if(name.back()!='/') {
            Range(offset,entry.offset,30); const auto* local=bytes_.data()+entry.offset;
            if(U32(local)!=0x04034b50 || U16(local+8)!=entry.method || U16(local+6)!=entry.flags) Invalid();
            const size_t localName=U16(local+26), header=30+localName+U16(local+28);
            Range(offset,entry.offset,header);
            if(Name(std::string(reinterpret_cast<const char*>(local+30),localName))!=name) Invalid();
            entry.offset+=uint32_t(header); Range(U32(e+16),entry.offset,entry.compressed);
            if(!entries_.emplace(name,entry).second) Invalid();
        }
        offset+=length;
    }
    if(offset!=centralEnd) Invalid();
    // A modern archive can carry classic fallback BMPs: reject its XML first.
    for(const auto& [name,entry]:entries_) {
        (void)entry;
        if(name=="skin.xml" || name.ends_with("/skin.xml"))
            throw std::runtime_error("Modern WAL skins are not supported in this version");
    }
    if(!entries_.contains("main.bmp")) {
        bool found=false;
        for(const auto& [name,entry]:entries_) {
            (void)entry;
            if(name.ends_with("/main.bmp")) { if(found) Invalid(); prefix_=name.substr(0,name.size()-8); found=true; }
        }
        if(!found) Invalid();
    }
}
bool Archive::Has(const std::string& name) const { return entries_.contains(prefix_+name); }
Bytes Archive::Read(const std::string& name) const {
    const auto found=entries_.find(prefix_+name); if(found==entries_.end()) return {};
    const auto& e=found->second; const auto* source=bytes_.data()+e.offset;
    Bytes result;
    if(e.method==0) { if(e.size!=e.compressed) Invalid(); result.assign(source,source+e.size); }
    else result=Inflate(source,e.compressed,e.size);
    if(Crc(result)!=e.crc) Invalid();
    return result;
}
}
