// Included inside Modern::Impl: all resources remain owned by one instance.
struct PrivateFont {
 Bytes bytes;Gdiplus::PrivateFontCollection collection;std::wstring family;
};
std::map<std::wstring,std::unique_ptr<PrivateFont>> privateFonts;
static std::wstring Time(int64_t ms) {
 const bool negative=ms<0;const auto seconds=std::abs(ms)/1000;wchar_t value[48]{};
 swprintf_s(value,L"%s%lld:%02lld",negative?L"-":L"",seconds/60,seconds%60);return value;
}
std::wstring TextValue(Node* n) {
 if(n->attrs.contains(L"alternatetext") && !n->attrs[L"alternatetext"].empty())return n->attrs[L"alternatetext"];
 const auto display=Lower(n->attrs[L"display"]);
 if(display==L"songname" || display==L"songtitle" || display==L"songinfo") {
  const auto status=HostStatus(host);if(!status.empty())return status;
 }
 if(n->attrs.contains(L"text"))return n->attrs[L"text"];
 auto state=State();
 if(display==L"time" || display==L"timeelapsed")return Time(state.position_ms);
 if(display==L"timeremaining")return Time(state.position_ms-state.duration_ms);
 if(display==L"songname" || display==L"songtitle")return std::wstring(state.title)+(n->Get(L"showlen") && state.duration_ms>0?L" ("+Time(state.duration_ms)+L")":L"");
 if(display==L"songinfo")return state.sample_rate?std::to_wstring(state.sample_rate/1000)+L" kHz "+std::to_wstring(state.bitrate/1000)+L" kbps":L"";
 if(display==L"bitrate")return state.bitrate?std::to_wstring(state.bitrate/1000):L"";
 if(display==L"samplerate")return state.sample_rate?std::to_wstring(state.sample_rate/1000):L"";
 return n->attrs[L"default"];
}
COLORREF ParseColor(const std::wstring& text,COLORREF fallback) {
 int r{},g{},b{};
 if(swscanf_s(text.c_str(),L"%d,%d,%d",&r,&g,&b)==3)return RGB(std::clamp(r,0,255),std::clamp(g,0,255),std::clamp(b,0,255));
 if(text.size()==7 && text[0]==L'#'){const auto c=wcstoul(text.c_str()+1,nullptr,16);return RGB(c>>16,(c>>8)&255,c&255);}
 return Color(Lower(text).c_str(),fallback);
}
std::wstring FontFamily(Node* n,Gdiplus::FontCollection*& collection) {
 const auto id=document.Alias(n->attrs[L"font"]);collection=nullptr;
 const auto found=document.fonts.find(id);if(found==document.fonts.end())return id.empty()?L"Tahoma":n->attrs[L"font"];
 if(found->second->kind!=L"truetypefont")return L"Tahoma";
 auto& font=privateFonts[id];
 if(!font) {
  font=std::make_unique<PrivateFont>();auto* resource=found->second;
  // Font::requestSkinFont falls back when a skin's TTF is absent/invalid.
  // Cache that result as well: do not retry the same missing file each paint.
  try {
   font->bytes=archive.Read(Narrow(ResourcePath(archive,resource->source,resource->attrs.at(L"file"))));
   Require(font->bytes.size()<=16*1024*1024 && font->collection.AddMemoryFont(font->bytes.data(),INT(font->bytes.size()))==Gdiplus::Ok,"invalid TrueType font");
   Gdiplus::FontFamily family;int count{};font->collection.GetFamilies(1,&family,&count);Require(count==1,"empty font resource");
   wchar_t name[LF_FACESIZE]{};family.GetFamilyName(name);font->family=name;
  }catch(const std::runtime_error&){font->family.clear();}
 }
 if(font->family.empty())return L"Arial";
 collection=&font->collection;return font->family;
}
void Image(Gdiplus::Graphics& g,Gdiplus::Bitmap* bitmap,const RECT& r,int alpha,const RECT* source=nullptr) {
 if(IsRectEmpty(&r) || alpha<=0)return;
 Gdiplus::ColorMatrix matrix={1,0,0,0,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,float(alpha)/255,0, 0,0,0,0,1};
 Gdiplus::ImageAttributes attributes;attributes.SetColorMatrix(&matrix);
 const RECT src=source?*source:RECT{0,0,LONG(bitmap->GetWidth()),LONG(bitmap->GetHeight())};
 g.DrawImage(bitmap,Gdiplus::Rect(r.left,r.top,r.right-r.left,r.bottom-r.top),src.left,src.top,src.right-src.left,src.bottom-src.top,Gdiplus::UnitPixel,&attributes);
}
RECT Geometry(Node* n,RECT parent,Gdiplus::Bitmap* image) {
 const int pw=parent.right-parent.left,ph=parent.bottom-parent.top;
 const auto relative=[&](const wchar_t* key,const wchar_t* relat,int size,int def) {
  const int v=n->Get(key,def),mode=n->Get(relat);return mode==1?size+v:mode==2?MulDiv(size,v,100):v;
 };
 int x=relative(L"x",L"relatx",pw,0),y=relative(L"y",L"relaty",ph,0);
 int w=relative(L"w",L"relatw",pw,image?image->GetWidth():n->Get(L"default_w",pw));
 int h=relative(L"h",L"relath",ph,image?image->GetHeight():n->Get(L"default_h",ph));
 if(n==layout){x=y=0;w=layout->Get(L"w");h=layout->Get(L"h");}
 if(const int fit=n->Get(L"fitparent")){x=y=std::max(0,-fit);w=pw-2*x;h=ph-2*y;}
 w=std::clamp(w,n->Get(L"minimum_w",0),std::max(n->Get(L"minimum_w",0),n->Get(L"maximum_w",8192)));
 h=std::clamp(h,n->Get(L"minimum_h",0),std::max(n->Get(L"minimum_h",0),n->Get(L"maximum_h",8192)));
 Require(w<=8192 && h<=8192 && std::abs(int64_t(x)+parent.left)<=65536 && std::abs(int64_t(y)+parent.top)<=65536,"computed layout bounds");
 return {parent.left+x,parent.top+y,parent.left+x+w,parent.top+y+h};
}
void Text(Gdiplus::Graphics& g,Node* n,RECT r,int alpha) {
 auto text=TextValue(n);if(text.empty() || IsRectEmpty(&r))return;
 if(n->Get(L"forceuppercase"))for(auto& ch:text)ch=towupper(ch);
 const auto savedText=g.Save();g.SetClip(Gdiplus::Rect(r.left,r.top,r.right-r.left,r.bottom-r.top),Gdiplus::CombineModeIntersect);
 const auto id=document.Alias(n->attrs[L"font"]);const auto resource=document.fonts.find(id);
 const auto align=Lower(n->attrs[L"align"]);
 const bool bitmapFont=resource!=document.fonts.end() && resource->second->kind==L"bitmapfont";
 if(bitmapFont && std::all_of(text.begin(),text.end(),[](wchar_t c){return c>=32 && c<127;})) {
  auto* font=resource->second;auto* bitmap=Bitmap(id);const int cw=_wtoi(font->attrs[L"charwidth"].c_str()),ch=_wtoi(font->attrs[L"charheight"].c_str());
  Require(cw>0 && ch>0 && cw<=256 && ch<=256,"bitmap font metrics");const int step=std::max(1,cw+_wtoi(font->attrs[L"hspacing"].c_str()));
  int x=r.left;if(align==L"right")x=r.right-int(text.size())*step;else if(align==L"center")x+=(r.right-r.left-int(text.size())*step)/2;
  // Wasabi BitmapFont::getXYfromChar: letters in row 0, digits/punctuation
  // in row 1. These atlases are not ASCII grids starting at character 32.
  for(wchar_t c:text) {
   int column=30,row=0;c=towupper(c);
   if(c>=L'A' && c<=L'Z')column=c-L'A';
   else if(c>=L'0' && c<=L'9'){column=c-L'0';row=1;}
   else {const std::wstring symbols=L"\1.:()-'!_+\\/[]~&%,=$#";const auto at=symbols.find(c);if(at!=std::wstring::npos){column=10+int(at);row=1;}else if(c==L'"')column=26;else if(c==L'@')column=27;else if(c==L'?'){column=3;row=2;}else if(c==L'*'){column=4;row=2;}}
   if(column*cw+cw<=int(bitmap->GetWidth()) && row*ch+ch<=int(bitmap->GetHeight()))g.DrawImage(bitmap,Gdiplus::Rect(x,r.top,cw,ch),column*cw,row*ch,cw,ch,Gdiplus::UnitPixel);
   x+=step;
  }
 } else {
  Gdiplus::FontCollection* collection{};const auto family=bitmapFont?std::wstring(L"Tahoma"):FontFamily(n,collection);
  const int style=(n->Get(L"bold")?Gdiplus::FontStyleBold:0)|(n->Get(L"italic")?Gdiplus::FontStyleItalic:0);
  Gdiplus::Font font(family.c_str(),float(std::clamp(n->Get(L"fontsize",12),1,256)),style,Gdiplus::UnitPixel,collection);
  Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericTypographic());format.SetFormatFlags(n->Get(L"wrap")?0:Gdiplus::StringFormatFlagsNoWrap);
  format.SetAlignment(align==L"right"?Gdiplus::StringAlignmentFar:align==L"center"?Gdiplus::StringAlignmentCenter:Gdiplus::StringAlignmentNear);
  format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
  const auto color=ParseColor(n->attrs[L"color"],RGB(255,255,255));
  const Gdiplus::RectF box(float(r.left+n->Get(L"offsetx")),float(r.top+n->Get(L"offsety")),float(r.right-r.left),float(r.bottom-r.top));
  g.SetTextRenderingHint(n->Get(L"antialias",1)?Gdiplus::TextRenderingHintAntiAliasGridFit:Gdiplus::TextRenderingHintSingleBitPerPixelGridFit);
  if(n->Get(L"shadowx") || n->Get(L"shadowy")) {
   const auto shadow=ParseColor(n->attrs[L"shadowcolor"],RGB(0,0,0));Gdiplus::SolidBrush brush(Gdiplus::Color(BYTE(alpha),GetRValue(shadow),GetGValue(shadow),GetBValue(shadow)));
   auto b=box;b.X+=n->Get(L"shadowx");b.Y+=n->Get(L"shadowy");g.DrawString(text.c_str(),int(text.size()),&font,b,&format,&brush);
  }
  Gdiplus::SolidBrush brush(Gdiplus::Color(BYTE(alpha),GetRValue(color),GetGValue(color),GetBValue(color)));g.DrawString(text.c_str(),int(text.size()),&font,box,&format,&brush);
 }
 g.Restore(savedText);
}
