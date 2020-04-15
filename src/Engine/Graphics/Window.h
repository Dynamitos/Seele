#pragma once
#include "GraphicsResources.h"
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
private:
    Array<PView> viewports;
    Gfx::PWindow gfxHandle;
};
DEFINE_REF(Window);
}