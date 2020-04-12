#include "SceneRenderPath.h"

Seele::SceneRenderPath::SceneRenderPath(Gfx::PGraphics graphics)
	: RenderPath(graphics)
{
}

Seele::SceneRenderPath::~SceneRenderPath()
{
}

void Seele::SceneRenderPath::applyArea(Rect newArea)
{
	area = newArea;
}

void Seele::SceneRenderPath::init()
{
}

void Seele::SceneRenderPath::beginFrame()
{
}

void Seele::SceneRenderPath::render()
{
}

void Seele::SceneRenderPath::endFrame()
{
}
