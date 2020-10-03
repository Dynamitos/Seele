#include "SceneView.h"
#include "SceneRenderPath.h"
#include "Scene/Scene.h"
#include "Window.h"
#include "Scene/Actor/CameraActor.h"

using namespace Seele;

Seele::SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo)
	: View(graphics, owner, createInfo)
{
	scene = new Scene(graphics);
	PCameraActor camera = new CameraActor();
	scene->addActor(camera);
	renderer = new SceneRenderPath(scene, graphics, viewport, camera);
}

Seele::SceneView::~SceneView()
{
}
