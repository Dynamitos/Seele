#include "WindowManager.h"

Seele::WindowManager::WindowManager()
{
}

Seele::WindowManager::~WindowManager()
{
}

void Seele::WindowManager::addWindow(const WindowCreateInfo& createInfo)
{
	PWindow window = new Window(createInfo);
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
