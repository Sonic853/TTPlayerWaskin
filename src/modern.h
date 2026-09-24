#pragma once
#include "skin.h"
#include <memory>
namespace waskin {
class Modern final:public Skin {
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    static Metadata Inspect(const wchar_t*);
    Modern(const wchar_t*,const TtpSkinHost*);
    ~Modern() override;
    const Metadata& Info() const override;
    std::wstring Diagnostic() const;
    HRESULT Attach(const TtpSkinWindows&) override;
    void Detach() noexcept override;
    HBITMAP Preview() override;
    void Paint(HWND,HDC,bool=false) override;
    void Shade() override {}
    bool Translate(const MSG&) override;
    HRESULT Layout(TtpSkinLayout&,bool) override;
    bool Handles(HWND) const override;
    bool PlaylistDrop(TtpSkinPlaylistDrop&) override;
    HMENU Menu(HWND,uint32_t) override;
    bool ContentState(TtpSkinContent&,bool) override;
};
}
