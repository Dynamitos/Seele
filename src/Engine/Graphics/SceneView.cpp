#include "SceneView.h"
#include "SceneRenderPath.h"

Seele::SceneView::SceneView()
{
}

Seele::SceneView::~SceneView()
{
}

void Seele::SceneView::initRenderer()
{
	renderer = new SceneRenderPath();
}
