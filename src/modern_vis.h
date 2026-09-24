#pragma once
#include "spectrum.h"

namespace waskin {
// SAWnd (sa.cpp) draws a 76x16 raster and exposes its left 72 columns.
// Only its presentation lives here; FFT and PCM come from the native player.
struct ModernVisStyle {
    std::array<COLORREF,24> palette{ClassicVisualPalette()};
    int falloff{2},peakFalloff{1},coloring{},oscStyle{1},fps{40};
    bool thin{},peaks{true},flipH{},flipV{};
    int channel{3};
    bool operator==(const ModernVisStyle&) const = default;
};
class ModernVis {
public:
    std::array<uint32_t,72*16> Render(const TtpSkinSpectrumFrame&,const ModernVisStyle&,DWORD);
private:
    ModernVisStyle style_{};
    std::array<int,75> bars_{},peaks_{},drawBars_{},drawPeaks_{};
    std::array<float,75> velocity_{};
    uint64_t generation_{};
    DWORD tick_{};
    uint32_t type_{};
    bool initialized_{};
};
}
