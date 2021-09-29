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
    windowView->renderGraph = view->renderGraph;
    windowView->worker = std::thread(&Window::viewWorker, this, windowView);
    views.add(windowView); 
}

void Window::render() 
{
    gfxHandle->beginFrame();
    for(auto& windowView : views)
    {
        UPViewFrame frame;
        {
            std::lock_guard lock(windowView->workerMutex);
            frame = std::move(windowView->currentFrame);
        }
        if(frame != nullptr)
        {
            windowView->renderGraph->render(std::move(frame));
        }
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
        windowView->view->beginFrame();
        windowView->view->render();
        windowView->view->endFrame();
        std::lock_guard lock(windowView->workerMutex);
        windowView->currentFrame = std::move(windowView->view->currentFrame);
    }
}
