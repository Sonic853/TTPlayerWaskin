// Winamp AnimatedLayer: atlas direction, frame range, pause/resume and events.
struct AnimationImage {Gdiplus::Bitmap* bitmap;RECT source;int count;};
AnimationImage Animation(Node* n) {
 const int elements=n->Get(L"elementframes");Require(elements>=0 && elements<=512,"animation element limit");
 if(elements) {
  const int index=n->animationFrame>=0 && n->animationFrame<elements?n->animationFrame:0;
  auto* bitmap=Bitmap(n->attrs[L"image"]+std::to_wstring(index));
  return {bitmap,{0,0,LONG(bitmap->GetWidth()),LONG(bitmap->GetHeight())},elements};
 }
 auto* bitmap=Bitmap(n->attrs[L"image"]);const int width=bitmap->GetWidth(),height=bitmap->GetHeight();
 const int fw=n->Get(L"framewidth",n->Get(L"w",width)),fh=n->Get(L"frameheight",n->Get(L"h",height));
 Require(fw>0 && fh>0,"animation frame dimensions");
 const bool horizontal=n->attrs.contains(L"framewidth") || (!n->attrs.contains(L"frameheight") && width!=fw);
 const int count=horizontal?width/fw:height/fh;Require(count>0 && count<=4096,"animation frame count");
 const int index=n->animationFrame>=0 && n->animationFrame<count?n->animationFrame:0;
 return {bitmap,horizontal?RECT{index*fw,0,(index+1)*fw,height}:RECT{0,index*fh,width,(index+1)*fh},count};
}
void GotoFrame(Node* n,int value) {
 if(n->animationFrame==value)return;n->animationFrame=value;Emit(n,L"onFrame",{double(value)});
}
Value AnimationCall(Node* n,const std::wstring& method,const std::vector<Value>& args) {
 if(method==L"getlength")return double(Animation(n).count);
 if(method==L"getcurframe")return double(n->animationFrame);
 if(method==L"getstartframe")return double(n->Get(L"start",0));
 if(method==L"getendframe")return double(n->Get(L"end",Animation(n).count-1));
 if(method==L"getdirection")return double(n->Get(L"start",0)<n->Get(L"end",Animation(n).count-1)?1:-1);
 if(method==L"getautoreplay")return double(n->Get(L"autoreplay",1)!=0);
 if(method==L"isplaying")return double(n->animationStatus==1);
 if(method==L"ispaused")return double(n->animationStatus==2);
 if(method==L"isstopped")return double(n->animationStatus==0);
 if(method==L"gotoframe")GotoFrame(n,int(std::clamp(Number(args.at(0)),-1.0,65535.0)));
 else if(method==L"setspeed"){n->attrs[L"speed"]=std::to_wstring(int(std::clamp(Number(args.at(0)),1.0,600000.0)));n->frameClock=GetTickCount();}
 else if(method==L"setstartframe" || method==L"setendframe") {
  const int value=int(std::clamp(Number(args.at(0)),-1.0,65535.0));if(value>=0)n->attrs[method==L"setstartframe"?L"start":L"end"]=std::to_wstring(value);
 }else if(method==L"setautoreplay")n->attrs[L"autoreplay"]=Number(args.at(0))?L"1":L"0";
 else if(method==L"setrealtime")n->attrs[L"realtime"]=Number(args.at(0))?L"1":L"0";
 else if(method==L"play"){GotoFrame(n,n->Get(L"start",0));n->frameClock=GetTickCount();n->animationStatus=1;Emit(n,L"onPlay");}
 else if(method==L"stop"){n->animationStatus=0;Emit(n,L"onStop");}
 else if(method==L"pause" || method==L"togglepause") {
  if(n->animationStatus==1){n->animationStatus=2;Emit(n,L"onPause");}
  else if(n->animationStatus==2){n->animationStatus=1;n->frameClock=GetTickCount();Emit(n,L"onResume");}
 }else throw std::runtime_error("unsupported AnimatedLayer method");return 0.0;
}
void AdvanceFrames(DWORD now) {
 // Bound callback work after a long suspension, while keeping normal skipped
 // timer steps and reverse ranges consistent with AnimatedLayer::timerCallback.
 const auto layers=nodes;for(auto* n:layers)if(!n->unsupported && n->kind==L"animatedlayer" && n->animationStatus==1) {
  const DWORD speed=DWORD(std::clamp(n->Get(L"speed",200),1,600000));
  const DWORD elapsed=now-n->frameClock;const unsigned steps=std::min<unsigned>(elapsed/speed,256);
  if(!steps)continue;n->frameClock=now-(elapsed%speed);
  for(unsigned i=0;i<steps && n->animationStatus==1;++i) {
   const int start=n->Get(L"start",0),end=n->Get(L"end",Animation(n).count-1);
   if(n->animationFrame==end) {
    if(!n->Get(L"autoreplay",1)){AnimationCall(n,L"stop",{});break;}
    n->animationFrame=start;
   }else GotoFrame(n,n->animationFrame+(start<end?1:-1));
  }
 }
}
