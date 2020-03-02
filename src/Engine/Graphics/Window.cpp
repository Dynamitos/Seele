#include "Window.h"
#include "Vulkan/VulkanGraphics.h"

Seele::Window::Window(const WindowCreateInfo& createInfo)
	: width(createInfo.width)
	, height(createInfo.height)
	, windowHandle(nullptr)
{
	graphics = new VulkanGraphics();
}

Seele::Window::~Window()
{
}
