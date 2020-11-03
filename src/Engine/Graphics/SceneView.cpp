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

void SceneView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier)
{
}

void SceneView::mouseMoveCallback(double xPos, double yPos) 
{
	static double prevXPos = 0.0f, prevYPos = 0.0f;
	double deltaX = xPos - prevXPos;
	double deltaY = yPos - prevYPos;
	prevXPos = xPos;
	prevYPos = yPos;
	activeCamera->getCameraComponent()->mouseMove(deltaX, deltaY);
}

void SceneView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) 
{
}
