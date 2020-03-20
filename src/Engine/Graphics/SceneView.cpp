#include "SceneView.h"
#include "SceneRenderPath.h"

Seele::SceneView::SceneView(Gfx::PGraphics graphics)
	: View(graphics)
{
	renderer = new SceneRenderPath(graphics);
}

Seele::SceneView::~SceneView()
{
}
