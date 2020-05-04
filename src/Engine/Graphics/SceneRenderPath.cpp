#include "SceneRenderPath.h"
#include "Scene/Scene.h"

Seele::SceneRenderPath::SceneRenderPath(Gfx::PGraphics graphics, Gfx::PViewport target)
	: RenderPath(graphics, target)
{
	scene = new Scene();
}

Seele::SceneRenderPath::~SceneRenderPath()
{
}
