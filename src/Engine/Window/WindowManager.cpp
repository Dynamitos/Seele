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
    OWindow window = new Window(this, graphics->createWindow(createInfo));
    PWindow ref = window;
    windows.add(std::move(window));
    return ref;
}

void WindowManager::notifyWindowClosed(PWindow window) 
{
    windows.remove(windows.find([window] (const OWindow& w) { return window == w; }));
    if(windows.empty())
    {
        exit(0);
    }
}
