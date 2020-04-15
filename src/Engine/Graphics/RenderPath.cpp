#include "RenderPath.h"

Seele::RenderPath::RenderPath(Gfx::PGraphics graphics, Gfx::PViewport target)
	: graphics(graphics)
	, target(target)
{
}

Seele::RenderPath::~RenderPath()
{
}

void Seele::RenderPath::applyArea(Rect newArea)
{
	target->resize(newArea.size.x, newArea.size.y);
	target->move(newArea.offset.x, newArea.offset.y);
}