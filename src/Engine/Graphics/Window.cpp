#include "Window.h"

Seele::Window::Window(Gfx::PWindow handle)
    : gfxHandle(handle)
{
}

Seele::Window::~Window()
{
    
}

void Seele::Window::addView(PView view)
{
    viewports.add(view);
}

void Seele::Window::beginFrame()
{
    gfxHandle->beginFrame();
    for(auto view : viewports)
    {
        view->beginFrame();
    }
}

void Seele::Window::render()
{
    for(auto view : viewports)
    {
        view->render();
    }
}

void Seele::Window::endFrame()
{
    gfxHandle->endFrame();
    for(auto view : viewports)
    {
        view->endFrame();
    }
}

Gfx::PWindow Seele::Window::getGfxHandle()
{
    return gfxHandle;
}