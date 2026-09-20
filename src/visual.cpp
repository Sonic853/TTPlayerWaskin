#include "skin.h"

namespace waskin {
void Skin::Visual(View& view,HDC dc,RECT bounds) const {
    const int saved=SaveDC(dc);
    IntersectClipRect(dc,bounds.left,bounds.top,bounds.right,bounds.bottom);
    SetDCBrushColor(dc,visual_colors_.background);
    FillRect(dc,&bounds,static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
    TtpSkinSpectrumFrame frame{};frame.size=sizeof(frame);
    if(host_.spectrum && host_.spectrum(host_.context,&frame) && frame.type==2) {
        const int scale=view.kind==1?1:scale_;
        const auto raster=view.spectrum.Render(frame,GetTickCount(),view.shaded,scale);
        std::array<uint32_t,152*32> pixels{};
        for(int i=0;i<raster.width*raster.height;++i) {
            const auto c=visual_palette_[raster.pixels[i]];
            pixels[i]=(uint32_t(GetRValue(c))<<16)|(uint32_t(GetGValue(c))<<8)|GetBValue(c);
        }
        BITMAPINFO info{};info.bmiHeader={sizeof(BITMAPINFOHEADER),raster.width,-raster.height,1,32,BI_RGB};
        SetStretchBltMode(dc,COLORONCOLOR);
        // Playlist takes the left 72 pixels of the normal analyzer; it does
        // not squeeze the full 76-pixel image into its smaller opening.
        StretchDIBits(dc,bounds.left,bounds.top,raster.width/scale,raster.height/scale,
            0,0,raster.width,raster.height,pixels.data(),&info,DIB_RGB_COLORS,SRCCOPY);
    } else {
        view.spectrum.Reset();
        if(host_.visual) host_.visual(host_.context,dc,&bounds,&visual_colors_);
    }
    RestoreDC(dc,saved);
}
}
