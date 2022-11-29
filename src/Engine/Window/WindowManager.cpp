#include "WindowManager.h"
#include "Graphics/Vulkan/VulkanGraphics.h"

using namespace Seele;

Gfx::PGraphics WindowManager::graphics;

WindowManager::WindowManager()
{
    graphics = new Vulkan::Graphics();
    GraphicsInitializer initializer;
    graphics->init(initializer);
}

WindowManager::~WindowManager()
{
}

PWindow WindowManager::addWindow(const WindowCreateInfo &createInfo)
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
