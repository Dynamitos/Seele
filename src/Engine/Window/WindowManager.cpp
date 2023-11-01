#include "WindowManager.h"
#include "Graphics/Graphics.h"

using namespace Seele;

WindowManager::WindowManager()
{
}

WindowManager::~WindowManager()
{
}

PWindow WindowManager::addWindow(Gfx::PGraphics graphics, const WindowCreateInfo &createInfo)
{
    Gfx::OWindow handle = graphics->createWindow(createInfo);
    OWindow window = new Window(this, handle);
    PWindow ref = window;
    windows.add(std::move(window));
    return ref;
}

void WindowManager::notifyWindowClosed(PWindow window) 
{
    windows.remove(windows.find([window] (OWindow w) { return window == w; }));
    if(windows.empty())
    {
        exit(0);
    }
}
