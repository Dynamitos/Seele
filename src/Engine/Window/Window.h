#pragma once
#include "Graphics/GraphicsResources.h"
#include "View.h"

namespace Seele
{
// The logical window, with the graphics proxy
class Window
{
public:
    Window(Gfx::PWindow handle);
    ~Window();
    void addView(PView view);
    void beginFrame();
    void render();
    void endFrame();
    Gfx::PWindow getGfxHandle();
    void setFocused(PView view);

private:
    Array<PView> viewports;
    PView focusedView;
    Gfx::PWindow gfxHandle;
};
DEFINE_REF(Window)
} // namespace Seele