#include "View.h"
#include "Window.h"
#include "Graphics/Graphics.h"

using namespace Seele;

View::View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &viewportInfo, std::string name)
	: graphics(graphics)
	, owner(window)
	, name(name)
{
	viewport = graphics->createViewport(owner->getGfxHandle(), viewportInfo);
	owner->addView(this);
	setFocused();
}

View::~View()
{
}

void View::setFocused() 
{
	owner->setFocused(this);	
}

const std::string& View::getName()
{
	return name;
}
