#pragma once
#include "Graphics/GraphicsResources.h"
#include "View.h"

namespace Seele
{
/*struct WindowView
{
    PView view;
    Event updateFinished;
    std::mutex workerMutex;
    WindowView(PView view)
        : view(view)
        , updateFinished(view->getName())
    {}
};*/
//DEFINE_REF(WindowView)
DECLARE_REF(WindowManager)
// The logical window, with the graphics proxy
class Window
{
public:
    Window(PWindowManager owner, Gfx::PWindow handle);
    ~Window();
    void addView(PView view);
    void render();
    Gfx::PWindow getGfxHandle();
    void setFocused(PView view);

protected:
    PWindowManager owner;
    Array<PView> views;
    Gfx::PWindow gfxHandle;

    //void viewWorker(size_t viewIndex);
};
DEFINE_REF(Window)
} // namespace Seele