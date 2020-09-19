#include "WindowManager.h"
#include "Vulkan/VulkanGraphics.h"

Gfx::PGraphics WindowManager::graphics;

Seele::WindowManager::WindowManager()
{
	graphics = new Vulkan::Graphics();
	GraphicsInitializer initializer;
	graphics->init(initializer);
	TextureCreateInfo info;
	info.width = 4096;
	info.height = 4096;
	Gfx::PTexture2D testTexture = graphics->createTexture2D(info);
	BulkResourceData resourceData;
	resourceData.size = 4096;
	resourceData.data = new uint8[4096];
	for (int i = 0; i < 4096; ++i)
	{
		resourceData.data[i] = (uint8)i;
	}
	Gfx::PUniformBuffer testUniform = graphics->createUniformBuffer(resourceData);
}

Seele::WindowManager::~WindowManager()
{
}

PWindow Seele::WindowManager::addWindow(const WindowCreateInfo &createInfo)
{
	Gfx::PWindow handle = graphics->createWindow(createInfo);
	PWindow window = new Window(handle);
	windows.add(window);
	return window;
}

void Seele::WindowManager::beginFrame()
{
	for (auto window : windows)
	{
		window->beginFrame();
	}
}

void WindowManager::render() 
{
	for(auto window : windows)
	{
		window->render();
	}	
}

void Seele::WindowManager::endFrame()
{
	for (auto window : windows)
	{
		window->endFrame();
	}
}
