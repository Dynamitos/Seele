#include "SceneView.h"
#include "SceneRenderPath.h"
#include "Scene/Scene.h"
#include "Window.h"

using namespace Seele;

Seele::SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo)
	: View(graphics, owner, createInfo)
{
	scene = new Scene(graphics);
	renderer = new SceneRenderPath(scene, graphics, viewport);
}

Seele::SceneView::~SceneView()
{
}
