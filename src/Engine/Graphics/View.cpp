#include "View.h"
#include "Window.h"

Seele::View::View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& viewportInfo)
	: graphics(graphics)
	, owner(owner)
{
	viewport = graphics->createViewport(owner->getGfxHandle(), viewportInfo);
}

Seele::View::~View()
{
}

void Seele::View::beginFrame()
{
	renderer->beginFrame();
}

void Seele::View::render()
{
	renderer->render();
}

void Seele::View::endFrame()
{
	renderer->endFrame();
}

void Seele::View::applyArea(Rect area)
{
	renderer->applyArea(area);
}
