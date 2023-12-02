#include "WindowManager.h"
#include "Graphics/Graphics.h"

using namespace Seele;

WindowManager::WindowManager()
{
}

WindowManager::~WindowManager()
{
}

OWindow WindowManager::addWindow(Gfx::PGraphics graphics, const WindowCreateInfo &createInfo)
{
    OWindow window = new Window(this, graphics->createWindow(createInfo));
    windows.add(window);
    return window;
}

void WindowManager::render()
{
    for (auto& window : windows)
    {
        window->render();
    }
}

void WindowManager::notifyWindowClosed(PWindow window) 
{
    windows.remove(window);
}
