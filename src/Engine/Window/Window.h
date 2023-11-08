#pragma once
#include "View.h"
#include "Graphics/RenderTarget.h"

namespace Seele
{
DECLARE_REF(WindowManager)
// The logical window, with the graphics proxy
class Window
{
public:
    Window(PWindowManager owner, Gfx::OWindow handle);
    ~Window();
    void addView(PView view);
    void render();
    Gfx::PWindow getGfxHandle();
    void setFocused(PView view);

protected:
    PWindowManager owner;
    Array<PView> views;
    Gfx::OWindow gfxHandle;

    //void viewWorker(size_t viewIndex);
};
DEFINE_REF(Window)
} // namespace Seele