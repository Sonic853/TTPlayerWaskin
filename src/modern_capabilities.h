#pragma once
#include "modern_objects.h"
#include "lazy_cache.h"
namespace waskin::modern {
struct ScriptClass {
    const wchar_t* guid;const wchar_t* kind;int parent;
    std::map<std::wstring,int> methods;
};
inline LazyCache<std::vector<ScriptClass>> scriptClasses;
inline const std::vector<ScriptClass>& Classes() {
    // GUIDs/inheritance: Wasabi/Lib/std.mi. Each import resolves on its own
    // class and ancestors; a Button never acquires Slider.setPosition.
    return scriptClasses.Get([] {return std::vector<ScriptClass>{
      {L"{51654971-0d87-4a51-91e3-a6b53235f3e7}",L"object",-1,{}},
      {L"{d6f50f64-93fa-49b7-93f1-ba66efae3e98}",L"system",0,{
       {L"onscriptloaded",0},{L"onscriptunloading",0},{L"onplay",0},{L"onpause",0},{L"onresume",0},{L"onstop",0},{L"ongetcancelcomponent",2},{L"onlookforcomponent",1},
       {L"getruntimeversion",0},{L"getskinname",0},{L"getprivateint",3},{L"setprivateint",3},{L"gettimeofday",0},{L"messagebox",4},{L"getscriptgroup",0},{L"getstatus",0},
       {L"getplayitemstring",0},{L"strleft",2},{L"getparam",0},{L"getsonginfotext",0},{L"gettoken",3},{L"strsearch",2},{L"onvolumechanged",1},{L"getposition",0},{L"getplayitemlength",0},{L"seekto",1},{L"integertotime",1},{L"getcontainer",1},{L"stringtointeger",1},{L"integertostring",1},{L"getvolume",0},{L"setvolume",1},{L"seteqband",2}}},
      {L"{4ee3e199-c636-4bec-97cd-78bc9c8628b0}",L"guiobject",0,{
       {L"findobject",1},{L"show",0},{L"hide",0},{L"getxmlparam",1},{L"setxmlparam",2},{L"settargetspeed",1},{L"settargetx",1},{L"gototarget",0},{L"ontargetreached",0},
       {L"onleftbuttondown",2},{L"onleftbuttonup",2},{L"onmousemove",2},{L"getalpha",0},{L"setalpha",1},{L"getleft",0},{L"gettop",0},{L"getwidth",0},{L"getheight",0}}},
      {L"{45be95e5-2072-4191-935c-bb5ff9f117fd}",L"group",2,{{L"getobject",1}}},
      {L"{60906d4e-537e-482e-b004-cc9461885672}",L"layout",3,{}},
      {L"{e90dc47b-840d-4ae7-b02c-040bd275f7fc}",L"container",0,{{L"getlayout",1}}},
      {L"{698eddcd-8f1e-4fec-9b12-f944f909ff45}",L"button",2,{{L"onleftclick",0},{L"leftclick",0},{L"setactivated",1},{L"getactivated",0},{L"onactivate",1}}},
      {L"{b4dccfff-81fe-4bcc-961b-720fd5be0fff}",L"togglebutton",6,{{L"ontoggle",1},{L"getcurcfgval",0},{L"getactivated",0},{L"setactivated",1}}},
      {L"{5ab9fa15-9a7d-4557-abc8-6557a6c67ca9}",L"layer",2,{{L"setregionfrommap",3}}},
      {L"{62b65e3f-375e-408d-8dea-76814ab91b77}",L"slider",2,{{L"setposition",1},{L"getposition",0},{L"onsetposition",1},{L"onpostedposition",1},{L"onsetfinalposition",1},{L"lock",0},{L"unlock",0}}},
      {L"{403abcc0-6f22-4bd6-8ba4-10c829932547}",L"component",2,{}},
      {L"{ce4f97be-77b0-4e19-9956-d49833c96c27}",L"vis",2,{}},
      {L"{efaa8672-310e-41fa-b7dc-85a9525bcb4b}",L"text",2,{{L"settext",1},{L"gettext",0},{L"setalternatetext",1}}},
      {L"{38603665-461b-42a7-aa75-d83f6667bf73}",L"map",0,{{L"loadmap",1},{L"getwidth",0},{L"getheight",0},{L"getvalue",2},{L"getargbvalue",3},{L"inregion",2}}},
      {L"{6b64cd27-5a26-4c4b-8c59-e6a70cf6493a}",L"animatedlayer",8,{
       {L"setspeed",1},{L"gotoframe",1},{L"setstartframe",1},{L"setendframe",1},{L"setautoreplay",1},{L"play",0},{L"pause",0},{L"togglepause",0},{L"stop",0},
       {L"isplaying",0},{L"ispaused",0},{L"isstopped",0},{L"getstartframe",0},{L"getendframe",0},{L"getlength",0},{L"getdirection",0},{L"getautoreplay",0},{L"getcurframe",0},
       {L"onplay",0},{L"onpause",0},{L"onresume",0},{L"onstop",0},{L"onframe",1},{L"setrealtime",1}}},
      {L"{5d0c5bb6-7de1-4b1f-a70f-8d1659941941}",L"timer",0,{{L"setdelay",1},{L"getdelay",0},{L"start",0},{L"stop",0},{L"isrunning",0},{L"ontimer",0}}}
    };});
}
inline int ClassIndex(const GUID& guid) {
    wchar_t text[40]{};StringFromGUID2(guid,text,40);const auto id=Lower(text);const auto& classes=Classes();
    for(size_t i=0;i<classes.size();++i)if(id==classes[i].guid)return int(i);return -1;
}
inline int MethodArity(const GUID& type,const std::wstring& name) {
    const auto& classes=Classes();int index=ClassIndex(type);Require(index>=0,"unsupported MAKI class");
    for(;index>=0;index=classes[index].parent)if(auto m=classes[index].methods.find(Lower(name));m!=classes[index].methods.end())return m->second;
    throw std::runtime_error("unsupported MAKI method on declared class");
}
inline bool SupportsClass(Node* node,const GUID& type) {
    const int target=ClassIndex(type);if(!node || target<0)return false;
    const auto& classes=Classes();int index=-1;
    for(size_t i=0;i<classes.size();++i)if(node->kind==classes[i].kind){index=int(i);break;}
    if(index<0 && node->kind==L"cfggroup")index=3;
    for(;index>=0;index=classes[index].parent)if(index==target)return true;
    return false;
}
}
