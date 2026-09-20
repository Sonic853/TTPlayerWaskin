#pragma once
#include "ttp_skin_plugin.h"
#include <array>

namespace waskin {
std::array<COLORREF,24> ClassicVisualPalette();

// The provider owns the classic pixels and decay. Audio/FFT stays in the host.
class ClassicSpectrum {
public:
    struct Raster {
        int width{},height{};
        std::array<unsigned char,152*32> pixels{}; // top-down palette indices
    };
    Raster Render(const TtpSkinSpectrumFrame& frame,DWORD now,bool shaded,int scale);
    void Reset() noexcept;
private:
    std::array<int,75> bars_{},peaks_{},draw_bars_{},draw_peaks_{};
    std::array<float,75> velocity_{};
    uint64_t generation_{};
    DWORD tick_{};
    int mode_{};
    bool initialized_{},active_{};
};
}
