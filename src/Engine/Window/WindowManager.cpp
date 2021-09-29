#include "WindowManager.h"
#include "Graphics/Vulkan/VulkanGraphics.h"

using namespace Seele;

Gfx::PGraphics WindowManager::graphics;

WindowManager::WindowManager()
{
	graphics = new Vulkan::Graphics();
	GraphicsInitializer initializer;
	graphics->init(initializer);
	TextureCreateInfo info;
	info.width = 4096;
	info.height = 4096;
	Gfx::PTexture2D testTexture = graphics->createTexture2D(info);
	UniformBufferCreateInfo uniformInitializer;
	uniformInitializer.resourceData.size = 4096;
	uniformInitializer.resourceData.data = new uint8[4096];
	for (int i = 0; i < 4096; ++i)
	{
		uniformInitializer.resourceData.data[i] = (uint8)i;
	}
	Gfx::PUniformBuffer testUniform = graphics->createUniformBuffer(uniformInitializer);
}

WindowManager::~WindowManager()
{
}

PWindow WindowManager::addWindow(const WindowCreateInfo &createInfo)
{
	Gfx::PWindow handle = graphics->createWindow(createInfo);
	PWindow window = new Window(handle);
	windows.add(window);
	return window;
}

void WindowManager::render() 
{
	for(auto window : windows)
	{
		window->render();
	}	
}

