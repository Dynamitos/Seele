#include "WindowManager.h"
#include "Vulkan/VulkanGraphics.h"

Seele::WindowManager::WindowManager()
{
	graphics = new VulkanGraphics();
	GraphicsInitializer initializer;
	graphics->init(initializer);
}

Seele::WindowManager::~WindowManager()
{
}

void Seele::WindowManager::addWindow(const WindowCreateInfo& createInfo)
{
	PWindow window = new Window(createInfo, graphics);
	windows.add(window);
}

void Seele::WindowManager::beginFrame()
{
	for (auto window : windows)
	{
		window->beginFrame();
	}
}

void Seele::WindowManager::endFrame()
{
	for (auto window : windows)
	{
		window->endFrame();
	}
}
