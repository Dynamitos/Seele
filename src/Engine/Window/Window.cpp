#include "Window.h"
#include <functional>

using namespace Seele;

Window::Window(Gfx::PWindow handle)
    : gfxHandle(handle)
{
}

Window::~Window()
{
}

void Window::addView(PView view)
{
    WindowView* windowView = new WindowView();
    windowView->view = view;
    //windowView->worker = std::thread(&Window::viewWorker, this, windowView);
    views.add(windowView); 
}

void Window::render() 
{
    gfxHandle->beginFrame();
    for(auto& windowView : views)
    {   
        windowView->view->beginUpdate();
        windowView->view->update();
        {
            std::unique_lock lock(windowView->workerMutex);
            windowView->view->commitUpdate();
        }
        windowView->updateFinished.raise();
        {
            std::unique_lock lock(windowView->workerMutex);
            windowView->view->prepareRender();
        }
        windowView->updateFinished.reset();
        windowView->view->render();
    }
    gfxHandle->endFrame();
}

Gfx::PWindow Window::getGfxHandle()
{
    return gfxHandle;
}
    
void Window::setFocused(PView view) 
{
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


void Window::viewWorker(WindowView* windowView)
{
    while(true)
    {
    }
}
