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
    Gfx::PWindow handle = graphics->createWindow(createInfo);
    PWindow window = new Window(this, handle);
    windows.add(window);
    return window;
}

void WindowManager::notifyWindowClosed(PWindow window) 
{
    windows.remove(windows.find(window));
    if(windows.empty())
    {
        exit(0);
    }
}
