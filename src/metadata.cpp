#include "metadata.h"
#include <windows.h>
#include <msxml6.h>
#include <cstring>
#include <memory>

namespace waskin {
namespace {
struct Release {void operator()(IUnknown* value) const {if(value) value->Release();}};
template<class T> using ComPtr=std::unique_ptr<T,Release>;
using Bstr=std::unique_ptr<OLECHAR,decltype(&SysFreeString)>;
struct Apartment {
    HRESULT result{CoInitializeEx(nullptr,COINIT_MULTITHREADED)};
    ~Apartment() {if(SUCCEEDED(result)) CoUninitialize();}
};
struct Variant {
    VARIANT value{};
    ~Variant() {VariantClear(&value);}
};
bool Named(IXMLDOMNode* node,const wchar_t* name) {
    BSTR raw{};if(FAILED(node->get_nodeName(&raw))) return false;
    const Bstr text(raw,SysFreeString);
    return text && _wcsicmp(text.get(),name)==0;
}
ComPtr<IXMLDOMNode> Child(IXMLDOMNode* parent,const wchar_t* name) {
    IXMLDOMNode* raw{};
    if(!parent || FAILED(parent->get_firstChild(&raw))) return {};
    ComPtr<IXMLDOMNode> node(raw);
    while(node) {
        DOMNodeType type{};
        if(SUCCEEDED(node->get_nodeType(&type)) && type==NODE_ELEMENT && Named(node.get(),name)) return node;
        IXMLDOMNode* next{};
        if(FAILED(node->get_nextSibling(&next))) break;
        node.reset(next);
    }
    return {};
}
std::wstring Field(IXMLDOMNode* parent,const wchar_t* name) {
    const auto node=Child(parent,name);if(!node) return {};
    BSTR raw{};if(FAILED(node->get_text(&raw))) return {};
    const Bstr text(raw,SysFreeString);if(!text) return {};
    std::wstring value(text.get(),SysStringLen(text.get()));
    const auto first=value.find_first_not_of(L" \t\r\n");
    return first==std::wstring::npos?std::wstring{}:value.substr(first,value.find_last_not_of(L" \t\r\n")-first+1);
}
}
Metadata ReadMetadata(const Bytes& bytes) {
    if(bytes.empty() || bytes.size()>256*1024) return {};
    Apartment apartment;
    if(FAILED(apartment.result) && apartment.result!=RPC_E_CHANGED_MODE) return {};
    IXMLDOMDocument2* raw{};
    if(FAILED(CoCreateInstance(__uuidof(DOMDocument60),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&raw)))) return {};
    ComPtr<IXMLDOMDocument2> document(raw);
    if(FAILED(document->put_async(VARIANT_FALSE)) ||
       FAILED(document->put_validateOnParse(VARIANT_FALSE)) ||
       FAILED(document->put_resolveExternals(VARIANT_FALSE))) return {};
    const Bstr property(SysAllocString(L"ProhibitDTD"),SysFreeString);
    VARIANT prohibit{};prohibit.vt=VT_BOOL;prohibit.boolVal=VARIANT_TRUE;
    if(!property || FAILED(document->setProperty(property.get(),prohibit))) return {};
    // Let MSXML decode the original byte stream, including BOMs and declared
    // legacy encodings. loadXML(BSTR) would lose that encoding information.
    Variant source;source.value.vt=VT_ARRAY|VT_UI1;
    source.value.parray=SafeArrayCreateVector(VT_UI1,0,static_cast<ULONG>(bytes.size()));
    if(!source.value.parray) return {};
    void* data{};if(FAILED(SafeArrayAccessData(source.value.parray,&data))) return {};
    std::memcpy(data,bytes.data(),bytes.size());SafeArrayUnaccessData(source.value.parray);
    VARIANT_BOOL loaded{};
    if(FAILED(document->load(source.value,&loaded)) || loaded!=VARIANT_TRUE) return {};
    IXMLDOMElement* element{};
    if(FAILED(document->get_documentElement(&element))) return {};
    ComPtr<IXMLDOMElement> root(element);if(!root) return {};
    ComPtr<IXMLDOMNode> wrapped;
    IXMLDOMNode* info{};
    // Match Winamp setup/skininfo.cpp's classic and wrapped SkinInfo roots.
    if(Named(root.get(),L"SkinInfo")) info=root.get();
    else if(Named(root.get(),L"WinampAbstractionLayer") || Named(root.get(),L"WasabiXML")) {
        wrapped=Child(root.get(),L"SkinInfo");info=wrapped.get();
    }
    if(!info) return {};
    return {Field(info,L"name"),Field(info,L"author"),Field(info,L"email"),Field(info,L"homepage")};
}
}
