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
    std::mutex workerMutex;
};
DEFINE_REF(WindowView)
DECLARE_REF(WindowManager)
// The logical window, with the graphics proxy
class Window
{
public:
    Window(PWindowManager owner, Gfx::PWindow handle);
    ~Window();
    void addView(PView view);
    MainJob render();
    Gfx::PWindow getGfxHandle();
    void setFocused(PView view);

protected:
    PWindowManager owner;
    Array<WindowView*> views;
    Gfx::PWindow gfxHandle;

    Job viewWorker(WindowView* view);
};
DEFINE_REF(Window)
} // namespace Seele