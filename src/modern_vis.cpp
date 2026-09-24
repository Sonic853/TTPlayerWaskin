#include "modern_vis.h"
#include <algorithm>
#include <cmath>

namespace waskin {
std::array<uint32_t,72*16> ModernVis::Render(const TtpSkinSpectrumFrame& f,const ModernVisStyle& s,DWORD now) {
    const DWORD interval=std::max(1,1000/std::clamp(s.fps,1,200));
    if(!initialized_ || generation_!=f.generation || type_!=f.type || !(style_==s)) {
        *this={};initialized_=true;generation_=f.generation;type_=f.type;style_=s;tick_=now-interval;
    }
    std::array<uint32_t,72*16> pixels{};
    pixels.fill(0xff000000);
    const auto put=[&](int x,int y,int color) {
        if(x<0 || x>=72 || y<0 || y>=16)return;
        if(s.flipH)x=71-x;if(s.flipV)y=15-y;
        const auto c=s.palette[std::clamp(color,0,23)];
        pixels[y*72+x]=0xff000000|(uint32_t(GetRValue(c))<<16)|(uint32_t(GetGValue(c))<<8)|GetBValue(c);
    };
    if(f.playback!=2 && f.playback!=3){bars_={};peaks_={};drawBars_={};drawPeaks_={};tick_=now;return pixels;}
    if(f.type==3 && f.sample_count>=300) {
        int previous=-1;
        for(int x=0;x<72;++x) {
            const int left=(s.channel&1)?f.samples_left[x*4]/256:0;
            const int right=(s.channel&2)?f.samples_right[x*4]/256:0;
            const int v=std::clamp(((left+right)>>4)+8,0,15);
            const int color=18+std::abs(v/2-4);
            if(s.oscStyle==0)put(x,v,color);
            else if(s.oscStyle==1)for(int y=std::min(previous<0?v:previous,v);y<=std::max(previous<0?v:previous,v);++y)put(x,y,color);
            else for(int y=std::min(7,v);y<=std::max(7,v);++y)put(x,y,color);
            previous=v;
        }
    }else if(f.type==2 && f.count>=256) {
        unsigned steps=std::min<DWORD>((now-tick_)/interval,8);
        if(f.playback==3){steps=0;tick_=now;}
        if(steps) {
            tick_=now-(now-tick_)%interval;
            std::array<int,76> values{};int previous=0;
            for(int x=0;x<75;++x) {
                const int bin=std::clamp(std::max(int(std::floor(std::pow(2.0,7.5*x/81.0))),previous+1),1,255);previous=bin;
                const int magnitude=std::max(0,int(f.magnitudes[bin]))>>5;
                values[x]=magnitude?std::clamp(int(std::log(double(magnitude))*16.0*1.3333/std::log(256.0)),0,15):0;
            }
            constexpr int falloff[]{3,6,12,16,32};constexpr float peakFalloff[]{1.05f,1.1f,1.2f,1.4f,1.6f};
            for(unsigned step=0;step<steps;++step)for(int x=0;x<75;++x) {
                const int t=x&~3;int v=s.thin?values[x]:(values[t]+values[t+1]+values[t+2]+values[t+3])/4;
                if((v<<4)<bars_[x])v=(bars_[x]-=falloff[std::clamp(s.falloff,0,4)])>>4;else bars_[x]=v<<4;
                bars_[x]=std::max(0,bars_[x]);v=std::max(0,v);
                if(peaks_[x]<=v*256){peaks_[x]=v*256;velocity_[x]=3.0f;}
                drawBars_[x]=v;drawPeaks_[x]=peaks_[x];
                peaks_[x]=std::max(0,peaks_[x]-int(velocity_[x]));velocity_[x]=std::min(4096.0f,velocity_[x]*peakFalloff[std::clamp(s.peakFalloff,0,4)]);
            }
        }
        for(int x=0;x<72;++x)if(s.thin || (x&3)!=3) {
            const int v=drawBars_[x],base=s.coloring==1?v+2:s.coloring==2?17-v:17;
            for(int y=0;y<v;++y)put(x,15-y,s.coloring==2?base:base-y);
            if(s.peaks)put(x,15-drawPeaks_[x]/256,23);
        }
    }
    return pixels;
}
}
