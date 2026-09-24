#pragma once
#include "archive.h"
#include <msxml6.h>
#include <gdiplus.h>
#include <objidl.h>
#include <map>
#include <memory>
#include <variant>
#include <functional>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <tuple>
namespace waskin::modern {
inline void Require(bool good,const char* message) {if(!good) throw std::runtime_error(message);}
inline std::wstring Wide(const std::string& s) {
    const int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),int(s.size()),nullptr,0);
    Require(count>0 || s.empty(),"invalid UTF-8");std::wstring out(count,L'\0');
    MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),int(s.size()),out.data(),count);return out;
}
inline std::wstring Lower(std::wstring s) {for(auto& c:s)c=towlower(c);return s;}
inline std::string Narrow(const std::wstring& s) {
    const int count=WideCharToMultiByte(CP_UTF8,0,s.data(),int(s.size()),nullptr,0,nullptr,nullptr);
    std::string out(count,'\0');WideCharToMultiByte(CP_UTF8,0,s.data(),int(s.size()),out.data(),count,nullptr,nullptr);return out;
}
using waskin::Bytes;
struct Release {void operator()(IUnknown* p) const {if(p)p->Release();}};
template<class T> using Com=std::unique_ptr<T,Release>;
struct Xml {
    std::wstring kind,source;std::map<std::wstring,std::wstring> attrs;std::vector<std::unique_ptr<Xml>> children;
};
inline std::unique_ptr<Xml> ReadXmlNode(IXMLDOMNode* node,size_t& total,unsigned depth=0) {
    Require(node && depth<64 && ++total<=8192,"XML tree limit");
    auto out=std::make_unique<Xml>();BSTR text{};node->get_nodeName(&text);out->kind=Lower(text?text:L"");SysFreeString(text);
    for(size_t at=0;(at=out->kind.find(L"_walcolon_",at))!=std::wstring::npos;)out->kind.replace(at,10,L":");
    IXMLDOMNamedNodeMap* raw{};node->get_attributes(&raw);Com<IXMLDOMNamedNodeMap> attributes(raw);
    if(attributes) {long count{};attributes->get_length(&count);for(long i=0;i<count;++i){
        IXMLDOMNode* item{};attributes->get_item(i,&item);Com<IXMLDOMNode> attr(item);
        BSTR key{},value{};attr->get_nodeName(&key);attr->get_text(&value);
        out->attrs[Lower(key?key:L"")]=value?value:L"";SysFreeString(key);SysFreeString(value);
    }}
    IXMLDOMNodeList* rawList{};node->get_childNodes(&rawList);Com<IXMLDOMNodeList> list(rawList);long count{};list->get_length(&count);
    for(long i=0;i<count;++i){IXMLDOMNode* item{};list->get_item(i,&item);Com<IXMLDOMNode> child(item);DOMNodeType type{};child->get_nodeType(&type);
        if(type==NODE_ELEMENT)out->children.push_back(ReadXmlNode(child.get(),total,depth+1));}
    return out;
}
inline std::unique_ptr<Xml> ParseXml(const Bytes& bytes) {
    Require(bytes.size()<=2*1024*1024,"XML size");
    auto text=Wide(std::string(bytes.begin(),bytes.end()));
    if(text.starts_with(L"\ufeff"))text.erase(0,1);
    if(text.starts_with(L"<?xml")){const auto end=text.find(L"?>");Require(end!=std::wstring::npos,"XML declaration");text.erase(0,end+2);}
    // Winamp's XML reader treats XUI names as opaque strings: names such as
    // Wasabi:StandardFrame:NoStatus are not XML namespace-qualified names.
    // Encode colons in element names only for MSXML, then restore them above.
    // Attribute values, comments, CDATA, resource IDs and scripts stay intact.
    for(size_t at=0;(at=text.find(L'<',at))!=std::wstring::npos;) {
        if(text.compare(at,4,L"<!--")==0){const auto end=text.find(L"-->",at+4);Require(end!=std::wstring::npos,"XML comment");at=end+3;continue;}
        if(text.compare(at,9,L"<![CDATA[")==0){const auto end=text.find(L"]]>",at+9);Require(end!=std::wstring::npos,"XML CDATA");at=end+3;continue;}
        size_t begin=at+1;if(begin<text.size() && text[begin]==L'/')++begin;
        if(begin>=text.size() || text[begin]==L'!' || text[begin]==L'?'){++at;continue;}
        const auto end=text.find_first_of(L" \t\r\n/>",begin);Require(end!=std::wstring::npos,"XML element name");
        auto name=text.substr(begin,end-begin);Require(Lower(name).find(L"_walcolon_")==std::wstring::npos,"reserved XML element name");
        for(size_t p=0;(p=name.find(L':',p))!=std::wstring::npos;p+=10)name.replace(p,1,L"_walcolon_");
        text.replace(begin,end-begin,name);at=begin+name.size();
    }
    text=L"<wal>"+text+L"</wal>";
    IXMLDOMDocument2* raw{};Require(SUCCEEDED(CoCreateInstance(__uuidof(DOMDocument60),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&raw))),"MSXML6 required for WAL");
    Com<IXMLDOMDocument2> doc(raw);doc->put_async(VARIANT_FALSE);doc->put_validateOnParse(VARIANT_FALSE);doc->put_resolveExternals(VARIANT_FALSE);
    VARIANT flag{};flag.vt=VT_BOOL;flag.boolVal=VARIANT_TRUE;BSTR property=SysAllocString(L"ProhibitDTD");doc->setProperty(property,flag);SysFreeString(property);
    BSTR source=SysAllocStringLen(text.data(),UINT(text.size()));VARIANT_BOOL loaded{};const auto result=doc->loadXML(source,&loaded);SysFreeString(source);
    Require(SUCCEEDED(result) && loaded==VARIANT_TRUE,"XML parse failed");IXMLDOMElement* root{};doc->get_documentElement(&root);Com<IXMLDOMElement> element(root);size_t total{};return ReadXmlNode(root,total);
}
struct Node {
    std::wstring kind,source;Node* parent{};std::map<std::wstring,std::wstring> attrs;std::vector<std::unique_ptr<Node>> children;
    std::unique_ptr<Gdiplus::Region> clipRegion;
    bool unsupported{},disabledAction{};
    int animationFrame{},animationStatus{};DWORD frameClock{};
    bool visible{true},animating{},animationStarted{},active{},locked{};int position{128},target{},origin{};DWORD start{};double speed{1};RECT bounds{};
    auto Save() const {return std::make_tuple(attrs,visible,animating,animationStarted,active,locked,position,target,origin,start,speed,bounds,animationFrame,animationStatus,frameClock,unsupported,disabledAction);}
    template<class T>void Restore(const T& state){std::tie(attrs,visible,animating,animationStarted,active,locked,position,target,origin,start,speed,bounds,animationFrame,animationStatus,frameClock,unsupported,disabledAction)=state;}
    std::wstring Id() const {const auto i=attrs.find(L"id");return i==attrs.end()?L"":i->second;}
    int Get(const wchar_t* key,int fallback=0) const {const auto i=attrs.find(key);return i==attrs.end()?fallback:_wtoi(i->second.c_str());}
    Node* Find(const std::wstring& id) {if(Lower(Id())==Lower(id))return this;for(auto& child:children)if(auto* n=child->Find(id))return n;return nullptr;}
};
using Value=std::variant<std::monostate,double,std::wstring,Node*>;
inline double Number(const Value& v) {if(auto p=std::get_if<double>(&v))return *p;throw std::runtime_error("expected numeric value");}
inline std::wstring String(const Value& v) {if(auto p=std::get_if<std::wstring>(&v))return *p;if(auto p=std::get_if<double>(&v))return std::to_wstring(int(*p));throw std::runtime_error("expected string");}
inline Node* Object(const Value& v) {const auto* p=std::get_if<Node*>(&v);Require(p && *p,"null/invalid object receiver");return *p;}

}
