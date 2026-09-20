#include "spectrum.h"
#include <algorithm>
#include <cmath>

namespace waskin {
std::array<COLORREF,24> ClassicVisualPalette() {
    return {RGB(0,0,0),RGB(24,24,41),RGB(239,49,16),RGB(206,41,16),
        RGB(214,90,0),RGB(214,102,0),RGB(214,115,0),RGB(198,123,8),
        RGB(222,165,24),RGB(214,181,33),RGB(189,222,41),RGB(148,222,33),
        RGB(41,206,16),RGB(50,190,16),RGB(57,181,16),RGB(49,156,8),
        RGB(41,148,0),RGB(24,132,8),RGB(255,255,255),RGB(214,214,222),
        RGB(181,189,189),RGB(160,170,175),RGB(148,156,165),RGB(150,150,150)};
}
void ClassicSpectrum::Reset() noexcept {*this={};}
ClassicSpectrum::Raster ClassicSpectrum::Render(const TtpSkinSpectrumFrame& frame,DWORD now,bool shaded,int scale) {
    scale=scale==2?2:1;
    const int mode=scale+(shaded?2:0);
    const bool available=frame.type==2 && frame.count>=256 && (frame.playback==2 || frame.playback==3);
    if(!initialized_ || generation_!=frame.generation || mode_!=mode || (!available && active_)) {
        Reset();initialized_=true;generation_=frame.generation;mode_=mode;tick_=now-16;
    }
    active_=available;
    const int columns=shaded && scale==1?37:75;
    unsigned steps=std::min<DWORD>((now-tick_)/16,8);
    if(!available || frame.playback==3) {tick_=now;steps=0;}
    if(steps) {
        tick_=now-(now-tick_)%16;
        // Keep the existing native FFT response, sampling 75 log-spaced
        // columns. Grouping, palette and motion below follow draw_sa.cpp.
        std::array<int,76> values{};int previous=0;
        for(int x=0;x<75;++x) {
            const int bin=std::clamp(std::max(int(std::floor(std::pow(2.0,7.5*x/81.0))),previous+1),1,255);
            previous=bin;
            const int magnitude=std::max(0,int(frame.magnitudes[bin]))>>5;
            values[x]=magnitude?std::clamp(int(std::log(double(magnitude))*16.0*1.3333/std::log(256.0)),0,255):0;
        }
        for(unsigned step=0;step<steps;++step) for(int x=0;x<columns;++x) {
            const int first=(x&~3)*(shaded && scale==1?2:1);
            int value=std::min(15,(values[first]+values[first+1]+values[first+2]+values[first+3])/4);
            // Winamp defaults: normal bars, 3px + 1px gap, falloff 12/16
            // per 16ms tick and peak velocity 3/256 with multiplier 1.1.
            if((value<<4)<bars_[x]) value=(bars_[x]-=12)>>4;
            else bars_[x]=value<<4;
            bars_[x]=std::max(0,bars_[x]);value=std::max(0,value);
            if(peaks_[x]<=value*256) {peaks_[x]=value*256;velocity_[x]=3.0f;}
            draw_bars_[x]=value;draw_peaks_[x]=peaks_[x];
            peaks_[x]=std::max(0,peaks_[x]-int(velocity_[x]));
            velocity_[x]=std::min(velocity_[x]*1.1f,4096.0f);
        }
    }
    Raster out;out.width=(shaded?38:76)*scale;out.height=(shaded?5:16)*scale;
    const auto put=[&](int x,int y,int color) {
        if(x>=0 && x<out.width && y>=0 && y<out.height) out.pixels[size_t(y)*out.width+x]=static_cast<unsigned char>(color);
    };
    if(!shaded) for(int y=0;y<out.height;++y) for(int x=0;x<out.width;++x)
        if((y/scale)%2 && (x/scale)%2==0) put(x,y,1);
    if(!available) return out;
    for(int x=0;x<columns;++x) {
        if((x&3)==3) continue;
        const int value=draw_bars_[x];
        if(shaded) {
            const int height=value*out.height/15;
            for(int row=0;row<height;++row) put(x,out.height-1-row,17-row*15/out.height);
            const int peak=draw_peaks_[x]*out.height/15/256;
            if(peak>=0 && peak<out.height) put(x,out.height-1-peak,23);
        } else {
            // Winamp's single-size draw uses row 14 of its bottom-up 32-row
            // DIB; the top 16-row copy clips its lowest two rows. Double-size
            // uses row 0, two pixels per point, and a two-pixel peak.
            const int bottom=scale==1?17:31;
            for(int row=0;row<value*scale;++row) for(int xx=0;xx<scale;++xx)
                put(x*scale+xx,bottom-row,17-row/scale);
            const int peak=draw_peaks_[x]/256;
            if(peak>=0 && peak<=15 && (scale==1 || peak>0))
                for(int yy=0;yy<scale;++yy) for(int xx=0;xx<scale;++xx) put(x*scale+xx,bottom-peak*scale-yy,23);
        }
    }
    return out;
}
}
