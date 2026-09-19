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
struct CursorHandle {
    HCURSOR value{};
    explicit CursorHandle(HCURSOR cursor):value(cursor) {}
    CursorHandle(CursorHandle&& other) noexcept:value(std::exchange(other.value,nullptr)) {}
    CursorHandle(const CursorHandle&)=delete;
    ~CursorHandle() {if(value) DestroyCursor(value);}
};
struct View {
    class Skin* skin{};
    HWND window{};
    int kind{};
    RECT saved{}, drag_rect{};
    HRGN saved_region{};
    std::vector<std::pair<HWND,bool>> children;
    POINT drag_start{};
    int pressed{}, scroll{}, selected{-1}, wheel{}, grab{}, seek{-1}, initial{};
    bool hot{true}, row_drag{}, selection_pending{};
    int drop{-1};
    bool shaded{}, dragging{}, resizing{}, host_drag{};
    int expanded_height{232};
};
Image MakeFallback(const char* name,int width,int height);
HCURSOR ReadCursor(const Bytes& bytes);
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
    std::unordered_map<std::string,Image> images_, fallback_;
    std::unordered_map<std::string,CursorHandle> cursors_;
    TtpSkinVisualColors visual_colors_{RGB(0,0,0),RGB(255,64,32),RGB(240,220,32),RGB(32,190,32),RGB(255,255,255),RGB(0,255,0)};
    int scale_{1}, row_height_{13};
    COLORREF text_color_{RGB(0,255,0)};
    unsigned feedback_until_{};
    std::wstring feedback_;
    uint32_t stats_index_{}, stats_count_{};
    int64_t stats_total_{}, stats_selected_{};
    bool stats_unknown_{}, stats_selected_unknown_{};
    std::wstring statistics_;
    void UpdateStatistics();
    void Feedback(const std::wstring& text);
    void Title(HDC dc,RECT bounds,const std::wstring& text) const;
    void Visual(HDC dc,RECT bounds) const;
    void ToggleScale();
    void BeginTrack(View& view,int hit,POINT point);
    void SelectRow(View& view,int row);
    HCURSOR Cursor(const View& view,POINT point) const;
    std::unordered_map<std::string,std::string> ini_;
    std::array<View,3> views_{};
    TtpSkinHost host_{};
    HFONT font_{};
    COLORREF normal_{RGB(0,255,0)}, current_{RGB(255,255,255)}, background_{RGB(0,0,0)}, selection_{RGB(0,0,198)};
    unsigned ticks_{};
    bool Sliding(int hit) const;
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
