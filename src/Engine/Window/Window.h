#pragma once
#include "Graphics/GraphicsResources.h"
#include "View.h"
#include "ThreadPool.h"

namespace Seele
{
struct WindowView
{
    PView view;
    Event updateFinished;
    std::thread worker;
    std::mutex workerMutex;
};
DEFINE_REF(WindowView)
// The logical window, with the graphics proxy
class Window
{
public:
    Window(Gfx::PWindow handle);
    ~Window();
    void addView(PView view);
    void render();
    Gfx::PWindow getGfxHandle();
    void setFocused(PView view);

protected:
    Array<WindowView*> views;
    Gfx::PWindow gfxHandle;

    void viewWorker(WindowView* view);
};
DEFINE_REF(Window)
} // namespace Seele