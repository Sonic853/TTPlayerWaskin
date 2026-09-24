 bool Content(TtpSkinContent& state) const {
  if(state.size<sizeof(state) || !window || state.window!=window)return false;
  state.bounds=contentRect;state.mode=contentMode;state.visual_type=contentVisual;return true;
 }
 void SyncContent(bool force=false) {
  if(!window || !host.content_input)return;
  if(!force && EqualRect(&contentRect,&notifiedContent))return;
  notifiedContent=contentRect;
  TtpSkinContent c{sizeof(c),window};Content(c);const MSG event{window,WM_SIZE};LRESULT result{};
  host.content_input(host.context,&c,&event,&result);
 }
 bool ContentInput(UINT message,WPARAM wp,LPARAM lp,LRESULT& result) {
  if(!host.content_input || moving || pressed)return false;
  TtpSkinContent c{sizeof(c),window};if(!Content(c))return false;
  POINT p{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
  if(message==WM_CONTEXTMENU && lp!=LPARAM(-1))ScreenToClient(window,&p);
  if((message==WM_RBUTTONUP || message==WM_CONTEXTMENU) &&
     !(message==WM_CONTEXTMENU && lp==LPARAM(-1)) && !PtInRect(&contentRect,p))return false;
  const MSG event{window,message,wp,lp};
  const bool handled=host.content_input(host.context,&c,&event,&result)!=FALSE;
  if((message==WM_RBUTTONUP || message==WM_CONTEXTMENU) && !IsRectEmpty(&contentRect)) {
   Command(TTP_SKIN_CONTENT_MENU,message==WM_CONTEXTMENU && lp==LPARAM(-1)?1:0);return true;
  }
  return handled;
 }
 COLORREF AttributeColor(Node* n,const std::wstring& name,COLORREF fallback)const {
  const auto i=n->attrs.find(name);if(i==n->attrs.end())return fallback;
  int r{},g{},b{};const auto& v=i->second;
  if(swscanf_s(v.c_str(),L"%d,%d,%d",&r,&g,&b)==3)return RGB(std::clamp(r,0,255),std::clamp(g,0,255),std::clamp(b,0,255));
  unsigned rgb{};if(!v.empty() && v[0]==L'#' && swscanf_s(v.c_str()+1,L"%x",&rgb)==1)return RGB((rgb>>16)&255,(rgb>>8)&255,rgb&255);
  return Color(Lower(v).c_str(),fallback);
 }
 ModernVisStyle VisStyle(Node* n)const {
  ModernVisStyle s;
  for(int i=0;i<16;++i)s.palette[17-i]=AttributeColor(n,L"colorband"+std::to_wstring(i+1),AttributeColor(n,L"colorallbands",s.palette[17-i]));
  for(int i=0;i<5;++i)s.palette[18+i]=AttributeColor(n,L"colorosc"+std::to_wstring(i+1),AttributeColor(n,L"colorallosc",s.palette[18+i]));
  s.palette[23]=AttributeColor(n,L"colorbandpeak",s.palette[23]);
  s.falloff=n->Get(L"falloff",2);s.peakFalloff=n->Get(L"peakfalloff",1);s.fps=n->Get(L"fps",40);
  s.peaks=n->Get(L"peaks",1)!=0;s.thin=Lower(n->attrs[L"bandwidth"])==L"thin";
  s.flipH=n->Get(L"fliph")!=0;s.flipV=n->Get(L"flipv")!=0;s.channel=n->Get(L"channel",3);
  const auto coloring=Lower(n->attrs[L"coloring"]),osc=Lower(n->attrs[L"oscstyle"]);
  s.coloring=coloring==L"fire"?1:coloring==L"line"?2:0;
  s.oscStyle=osc==L"dots"?0:osc==L"lines"?2:1;return s;
 }
