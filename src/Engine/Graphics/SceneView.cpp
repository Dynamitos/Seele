#include "SceneView.h"
#include "SceneRenderPath.h"

Seele::SceneView::SceneView(Graphics* graphics)
	: View(graphics)
{
	renderer = new SceneRenderPath(graphics);
}

Seele::SceneView::~SceneView()
{
}
