#include "Window.h"
#include <functional>

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
    for (auto view : viewports)
    {
        view->beginFrame();
    }
}

void Seele::Window::render()
{
    for (auto view : viewports)
    {
        view->render();
    }
}

void Seele::Window::endFrame()
{
    gfxHandle->endFrame();
    for (auto view : viewports)
    {
        view->endFrame();
    }
}

Gfx::PWindow Seele::Window::getGfxHandle()
{
    return gfxHandle;
}
    
void Window::setFocused(PView view) 
{
    focusedView = view;
    auto keyFunction = std::bind(&View::keyCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto mouseMoveFunction = std::bind(&View::mouseMoveCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    auto mouseButtonFunction = std::bind(&View::mouseButtonCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto scrollFunction = std::bind(&View::scrollCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    auto fileFunction = std::bind(&View::fileCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    gfxHandle->setKeyCallback(keyFunction);
    gfxHandle->setMouseMoveCallback(mouseMoveFunction);
    gfxHandle->setMouseButtonCallback(mouseButtonFunction);
    gfxHandle->setScrollCallback(scrollFunction);
    gfxHandle->setFileCallback(fileFunction);
}
