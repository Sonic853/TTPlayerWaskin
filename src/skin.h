#pragma once
#include "archive.h"
#include "ttp_skin_plugin.h"
#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <commctrl.h>

namespace waskin {
struct Image {
    HBITMAP bitmap{};
    int width{}, height{};
    Image() = default;
    explicit Image(const Bytes& bytes);
    ~Image() { if(bitmap) DeleteObject(bitmap); }
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&& other) noexcept : bitmap(std::exchange(other.bitmap,nullptr)), width(other.width),height(other.height) {}
};
struct View {
    class Skin* skin{};
    HWND window{};
    int kind{};
    RECT saved{}, drag_rect{};
    HRGN saved_region{};
    std::vector<std::pair<HWND,bool>> children;
    POINT drag_start{};
    int pressed{}, scroll{}, selected{-1}, wheel{};
    bool shaded{}, dragging{}, resizing{}, host_drag{};
    int expanded_height{232};
};
class Skin {
public:
    Skin(const wchar_t* path,const TtpSkinHost* host);
    ~Skin();
    HRESULT Attach(const TtpSkinWindows& windows);
    void Detach() noexcept;
    HBITMAP Preview();
    void Paint(HWND window,HDC dc);
    void Shade();
    bool Translate(const MSG& message);
    static LRESULT CALLBACK Subclass(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
private:
    std::unordered_map<std::string,Image> images_;
    std::unordered_map<std::string,std::string> ini_;
    std::array<View,3> views_{};
    TtpSkinHost host_{};
    HFONT font_{};
    COLORREF normal_{RGB(0,255,0)}, current_{RGB(255,255,255)}, background_{RGB(0,0,0)}, selection_{RGB(0,0,128)};
    unsigned ticks_{};
    TtpSkinState State() const;
    void Command(uint32_t command,int32_t value=0) const;
    bool Blit(HDC dc,const char* image,int x,int y,int w,int h,int sx=0,int sy=0,int sw=0,int sh=0) const;
    void Text(HDC dc,RECT bounds,const std::wstring& text,COLORREF color,bool bitmap=false) const;
    void Draw(View& view,HDC dc,int width,int height);
    void DrawMain(View& view,HDC dc,const TtpSkinState& state);
    void DrawPlaylist(View& view,HDC dc,int width,int height,const TtpSkinState& state);
    void DrawEqualizer(View& view,HDC dc,const TtpSkinState& state);
    int Hit(const View& view,POINT point) const;
    void Activate(View& view,int hit,POINT point);
    void Track(View& view,int hit,POINT point);
    void ToggleShade(View& view);
    void Region(View& view);
    void HideChildren(View& view);
    bool HostDrag(View& view, uint32_t phase, POINT point = {}) const;
    void EndDrag(View& view);
    LRESULT Message(View& view,UINT message,WPARAM wp,LPARAM lp);
};
}
