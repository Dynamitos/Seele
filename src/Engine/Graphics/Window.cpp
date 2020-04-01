#include "Window.h"
#include "SceneView.h"

using namespace Seele;

Window::Window(const WindowCreateInfo& createInfo, Gfx::PGraphics graphics)
	: width(createInfo.width)
	, height(createInfo.height)
	, graphics(graphics)
{
	center = new Section();
	center->resizeArea(Rect(1, 1, 0, 0));
	center->addView(new SceneView(graphics));
	windowHandle = graphics->createWindow(createInfo);
}

Window::~Window()
{
}

void Window::onWindowCloseEvent()
{
	
}

void Window::beginFrame()
{
	graphics->beginFrame(windowHandle);
	center->beginFrame();
}

void Window::endFrame()
{
	graphics->endFrame(windowHandle);
}
