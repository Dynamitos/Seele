#pragma once
#include "Graphics/RenderTarget.h"
#include "View.h"

namespace Seele {
DECLARE_REF(WindowManager)
// The logical window, with the graphics proxy
class Window {
  public:
    Window(PWindowManager owner, Gfx::OWindow handle);
    ~Window();
    void addView(PView view);
    void pollInputs();
    void render();
    Gfx::PWindow getGfxHandle();
    void setFocused(PView view);
    void onResize(uint32 width, uint32 height);
    constexpr bool isPaused() const { return gfxHandle->isPaused(); }

  protected:
    PWindowManager owner;
    Array<PView> views;
    Gfx::OWindow gfxHandle;
};
DEFINE_REF(Window)
} // namespace Seele