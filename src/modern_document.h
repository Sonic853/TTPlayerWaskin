#pragma once
#include "modern_objects.h"
#include <set>

namespace waskin::modern {
struct MissingResource:std::runtime_error {using std::runtime_error::runtime_error;};
struct CompatibilityNotes {
    std::vector<std::string> items;
    void Add(const std::string& text) {if(items.size()<64 && std::find(items.begin(),items.end(),text)==items.end())items.push_back(text);}
    std::wstring Text()const {std::wstring out;for(const auto& item:items){if(!out.empty())out+=L"\r\n";out+=Wide(item);}return out;}
};
// Package paths stay inside the archive. Winamp skins use both include-local
// and skin-root resource references; prefer the local path when it exists.
inline std::wstring PackagePath(std::wstring path) {
    std::replace(path.begin(),path.end(),L'\\',L'/');
    Require(!path.empty() && path.front()!=L'/' && path.find(L':')==std::wstring::npos,"absolute WAL resource path");
    std::vector<std::wstring> parts;
    for(size_t start=0;start<path.size();) {
        const auto end=path.find(L'/',start);const auto part=path.substr(start,end==std::wstring::npos?end:end-start);
        if(part==L"..") {Require(!parts.empty(),"WAL path escapes package");parts.pop_back();}
        else if(!part.empty() && part!=L".")parts.push_back(Lower(part));
        if(end==std::wstring::npos)break;start=end+1;
    }
    std::wstring out;for(const auto& part:parts){if(!out.empty())out+=L'/';out+=part;}
    Require(!out.empty(),"empty WAL resource path");return out;
}
inline std::wstring ResourcePath(const Archive& archive,const std::wstring& source,const std::wstring& file) {
    if(file.find(L'@')!=std::wstring::npos)throw MissingResource("external/system XML resource required: path variable");
    const auto slash=source.find_last_of(L'/');
    const auto local=PackagePath((slash==std::wstring::npos?L"":source.substr(0,slash+1))+file);
    if(archive.Has(Narrow(local)))return local;
    const auto root=PackagePath(file);if(archive.Has(Narrow(root)))return root;
    throw MissingResource(Narrow(source+L": missing resource "+file));
}
struct Document {
    const Archive& archive;
    CompatibilityNotes notes;
    bool partial{};
    std::unique_ptr<Xml> root;
    std::map<std::wstring,Xml*> definitions,xui,bitmaps,fonts,aliases;
    std::map<Xml*,Xml*> ancestors;
    std::map<Xml*,size_t> order;
    std::vector<Xml*> elements;
    std::set<std::wstring> loading;
    size_t files{},count{};
    explicit Document(const Archive& a,bool allowPartial=false):archive(a),partial(allowPartial) {root=Read(L"skin.xml");Collect(root.get());}
    std::unique_ptr<Xml> Read(const std::wstring& path) {
        Require(++files<=128 && loading.size()<32,"WAL include limit");
        Require(loading.insert(path).second,"cyclic WAL include");
        std::unique_ptr<Xml> xml;
        try {xml=ParseXml(archive.Read(Narrow(path)));Expand(xml.get(),path);}
        catch(const std::exception& e){loading.erase(path);throw std::runtime_error(Narrow(path)+": "+e.what());}
        loading.erase(path);return xml;
    }
    void Expand(Xml* node,const std::wstring& source) {
        Require(++count<=32768,"WAL expanded XML limit");node->source=source;
        if(node->kind==L"include") {
            const auto file=node->attrs.find(L"file");Require(file!=node->attrs.end(),"include missing file");
            std::wstring path;
            try {path=ResourcePath(archive,source,file->second);}
            catch(const MissingResource& e) {if(!partial)throw;notes.Add(Narrow(source)+": skipped include: "+e.what());node->kind=L"fragment";node->children.clear();return;}
            auto included=Read(path);
            node->kind=L"fragment";node->children=std::move(included->children);return;
        }
        for(auto& child:node->children)Expand(child.get(),source);
    }
    void Collect(Xml* n) {
        order[n]=elements.size();elements.push_back(n);
        const auto id=n->attrs.find(L"id");
        if(n->kind==L"groupdef") {
            Require(id!=n->attrs.end(),"groupdef missing id");const auto key=Lower(id->second);
            if(auto it=definitions.find(key);it!=definitions.end())ancestors[n]=it->second;
            definitions[key]=n;
            if(auto it=n->attrs.find(L"xuitag");it!=n->attrs.end())xui[Lower(it->second)]=n;
        }else if(id!=n->attrs.end()) {
            if(n->kind==L"bitmap")bitmaps[Lower(id->second)]=n;
            if(n->kind==L"bitmapfont" || n->kind==L"truetypefont")fonts[Lower(id->second)]=n;
            if(n->kind==L"elementalias")aliases[Lower(id->second)]=n;
        }
        for(auto& child:n->children)Collect(child.get());
    }
    Xml* DefinitionAt(const std::wstring& id,size_t position) const {
        const auto it=definitions.find(Lower(id));if(it==definitions.end())return nullptr;
        Xml* current=it->second;
        // Static layouts are instantiated as the XML stream is read. A later
        // layout's repeated groupdef must not replace an earlier layout's
        // buttons (Objection uses the same IDs with opposite SWITCH actions).
        while(order.at(current)>position && ancestors.contains(current))current=ancestors.at(current);
        return current;
    }
    std::wstring Alias(std::wstring id) const {
        std::set<std::wstring> seen;id=Lower(id);
        while(aliases.contains(id)) {
            Require(seen.insert(id).second && seen.size()<32,"cyclic resource alias");
            id=Lower(aliases.at(id)->attrs.at(L"target"));
        }
        return id;
    }
};
}
