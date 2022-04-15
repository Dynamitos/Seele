#include "Window.h"
#include "WindowManager.h"
#include <functional>

using namespace Seele;

Window::Window(PWindowManager owner, Gfx::PWindow handle)
    : owner(owner)
    , gfxHandle(handle)
{
}

Window::~Window()
{
}

void Window::addView(PView view)
{
    //WindowView* windowView = new WindowView(view);
    //windowView->worker = std::thread(&Window::viewWorker, this, windowView);
    views.add(view);
    //viewWorker(views.size() - 1);
}

void Window::render() 
{
    while(owner->isActive())
    {
        gfxHandle->beginFrame();
        for(auto& view : views)
        {
            view->beginUpdate();
            view->update();
            view->commitUpdate();
            view->prepareRender();
            view->render();
        }
        gfxHandle->endFrame();
        if(owner->isActive())
        {
            render();
        }
    }
    //co_return;
}

Gfx::PWindow Window::getGfxHandle()
{
    return gfxHandle;
}
    
void Window::setFocused(PView view) 
{
    auto keyFunction = std::bind(&View::keyCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto mouseMoveFunction = std::bind(&View::mouseMoveCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    auto mouseButtonFunction = std::bind(&View::mouseButtonCallback, view.getHandle() , std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto scrollFunction = std::bind(&View::scrollCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    auto fileFunction = std::bind(&View::fileCallback, view.getHandle(), std::placeholders::_1, std::placeholders::_2);
    gfxHandle->setKeyCallback(keyFunction);
    gfxHandle->setMouseMoveCallback(mouseMoveFunction);
    gfxHandle->setMouseButtonCallback(mouseButtonFunction);
    gfxHandle->setScrollCallback(scrollFunction);
    gfxHandle->setFileCallback(fileFunction);
    gfxHandle->setCloseCallback([this](){
        owner->notifyWindowClosed(this);
    });
}

/*void Window::viewWorker(size_t viewIndex)
{
    WindowView* windowView = views[viewIndex];
    co_await windowView->view->beginUpdate();
    co_await windowView->view->update();
    {
        std::scoped_lock lock(windowView->workerMutex);
        windowView->view->commitUpdate();
    }
    //std::cout << "Update completed" << std::endl;
    //windowView->updateFinished.raise();
    // enqueue next frame update
    if(owner->isActive())
    {
        viewWorker(viewIndex);
    }
    //co_return;
}*/
