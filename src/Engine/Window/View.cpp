#include "View.h"
#include "Window.h"
#include "Graphics/Graphics.h"

using namespace Seele;

View::View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &viewportInfo)
	: graphics(graphics), owner(window)
{
	viewport = graphics->createViewport(owner->getGfxHandle(), viewportInfo);
}

View::~View()
{
}

void View::applyArea(URect)
{

}

void View::setFocused() 
{
	owner->setFocused(this);	
}
