Value MapCall(Node* n,const std::wstring& method,const std::vector<Value>& args) {
 if(method==L"loadmap"){const auto id=String(args.at(0));Bitmap(id);Require(!missingImages.contains(document.Alias(id)),"script map is missing");n->attrs[L"map"]=id;return 0.0;}
 auto* bitmap=n->attrs[L"map"].empty()?nullptr:Bitmap(n->attrs[L"map"]);
 if(method==L"getwidth")return double(bitmap?bitmap->GetWidth():0);
 if(method==L"getheight")return double(bitmap?bitmap->GetHeight():0);
 if(method==L"getvalue" || method==L"getargbvalue" || method==L"inregion") {
  const int x=int(Number(args.at(0))),y=int(Number(args.at(1)));Gdiplus::Color color(0,0,0,0);
  if(bitmap && x>=0 && y>=0 && x<int(bitmap->GetWidth()) && y<int(bitmap->GetHeight()))bitmap->GetPixel(x,y,&color);
  if(method==L"inregion")return double(color.GetA()!=0);
  // SkinBitmap's Map::getValue reads premultiplied channels. getARGBValue
  // exposes straight channels (GetPixel already returns those in GDI+).
  if(method==L"getvalue")return double(std::max({color.GetR(),color.GetG(),color.GetB()})*color.GetA()/255);
  const int channel=int(Number(args.at(2)));Require(channel>=0,"negative Map channel");
  return double((color.GetValue()>>(8*(channel%4)))&255);
 }
 throw std::runtime_error("unsupported Map method");
}
void RegionFromMap(Node* n,Node* map,int threshold,bool inverse) {
 Require(map->kind==L"map" && !map->attrs[L"map"].empty(),"region requires a loaded Map");
 auto* bitmap=Bitmap(map->attrs[L"map"]);const int width=bitmap->GetWidth(),height=bitmap->GetHeight();
 Require(uint64_t(width)*height<=4*1024*1024,"Map region size limit");
 Gdiplus::BitmapData data{};Gdiplus::Rect bounds(0,0,width,height);
 Require(bitmap->LockBits(&bounds,Gdiplus::ImageLockModeRead,PixelFormat32bppPARGB,&data)==Gdiplus::Ok,"Map pixels");
 struct Unlock {Gdiplus::Bitmap* bitmap;Gdiplus::BitmapData* data;~Unlock(){bitmap->UnlockBits(data);}} unlock{bitmap,&data};
 std::vector<RECT> spans;
 const auto included=[&](uint32_t c){const int b=c&255,g=(c>>8)&255,r=(c>>16)&255;return (c>>24)!=0 && (inverse?std::max({b,g,r})<=threshold:std::min({b,g,r})>=threshold);};
 for(int y=0;y<height;++y) {
  auto* row=reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(data.Scan0)+y*data.Stride);
  for(int x=0;x<width;){while(x<width && !included(row[x]))++x;const int begin=x;while(x<width && included(row[x]))++x;if(x>begin){Require(spans.size()<65536,"Map region complexity");spans.push_back({begin,y,x,y+1});}}
 }
 auto region=std::make_unique<Gdiplus::Region>();region->MakeEmpty();
 for(const auto& r:spans)region->Union(Gdiplus::Rect(r.left,r.top,r.right-r.left,r.bottom-r.top));
 n->clipRegion=std::move(region);
}
bool MouseEvent(Node* n,const wchar_t* name,POINT point) {
 if(!n)return false;completed=false;
 // Virtual GUI objects receive coordinates in their parent group's client.
 const auto parent=n->parent?n->parent->bounds:RECT{};
 Emit(n,name,{double(point.x-parent.left),double(point.y-parent.top)});return completed;
}
