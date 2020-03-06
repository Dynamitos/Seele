#include "Window.h"
#include "Vulkan/VulkanGraphics.h"
#include "SceneView.h"

Seele::Window::Window(const WindowCreateInfo& createInfo)
	: width(createInfo.width)
	, height(createInfo.height)
{
	graphics = new VulkanGraphics();
	center = new Section();
	center->resizeArea(Rect(1, 1, 0, 0));
	center->addView(new SceneView(graphics));
	windowHandle = graphics->createWindow(createInfo);
}

Seele::Window::~Window()
{
}

void Seele::Window::beginFrame()
{
	graphics->beginFrame(windowHandle);
	center->beginFrame();
}

void Seele::Window::endFrame()
{
	graphics->endFrame(windowHandle);
}
