#include "View.h"
#include "Window.h"
#include "Graphics/Graphics.h"

Seele::View::View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &viewportInfo)
	: graphics(graphics), owner(window)
{
	viewport = graphics->createViewport(owner->getGfxHandle(), viewportInfo);
}

Seele::View::~View()
{
}

void Seele::View::beginFrame()
{
}

void Seele::View::render()
{
}

void Seele::View::endFrame()
{

}

void Seele::View::applyArea(URect area)
{

}

void View::setFocused() 
{
	owner->setFocused(this);	
}
