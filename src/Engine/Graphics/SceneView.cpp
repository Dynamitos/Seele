#include "SceneView.h"
#include "SceneRenderPath.h"
#include "Scene/Scene.h"
#include "Window.h"
#include "Scene/Actor/CameraActor.h"
#include "Scene/Components/CameraComponent.h"
#include <iostream>

using namespace Seele;

Seele::SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo)
	: View(graphics, owner, createInfo)
{
	scene = new Scene(graphics);
	activeCamera = new CameraActor();
	scene->addActor(activeCamera);
	renderer = new SceneRenderPath(scene, graphics, viewport, activeCamera);
}

Seele::SceneView::~SceneView()
{
}

void SceneView::keyCallback(KeyCode code, KeyAction action, KeyModifier modifier)
{
	std::cout << "Key callback " << (uint32)code << std::endl;
	activeCamera->getCameraComponent()->moveOrigin(1);
}
