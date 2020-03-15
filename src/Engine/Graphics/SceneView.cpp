#include "SceneView.h"
#include "SceneRenderPath.h"

Seele::SceneView::SceneView(PGraphics graphics)
	: View(graphics)
{
	renderer = new SceneRenderPath(graphics);
}

Seele::SceneView::~SceneView()
{
}
