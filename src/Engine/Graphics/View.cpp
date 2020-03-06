#include "View.h"

Seele::View::View(Graphics* graphics)
	: graphics(graphics)
{
}

Seele::View::~View()
{
}

void Seele::View::beginFrame()
{
	renderer->beginFrame();
}

void Seele::View::endFrame()
{
	renderer->endFrame();
}

void Seele::View::applyArea(Rect area)
{
	renderer->applyArea(area);
}
