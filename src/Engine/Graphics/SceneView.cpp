#include "SceneView.h"
#include "ForwardPlusRenderPath.h"

Seele::SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo& createInfo)
	: View(graphics, owner, createInfo)
{
	renderer = new ForwardPlusRenderPath(graphics, viewport);
}

Seele::SceneView::~SceneView()
{
}
