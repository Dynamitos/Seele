#include "SceneView.h"
#include "SceneRenderPath.h"
#include "Scene/Scene.h"
#include "Window.h"
#include "Scene/Actor/CameraActor.h"
#include "Scene/Components/CameraComponent.h"

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

void SceneView::beginFrame() 
{
	View::beginFrame();
	scene->tick(Gfx::currentFrameDelta);//TODO: update in separate thread
}

void SceneView::keyCallback(KeyCode code, InputAction action, KeyModifier)
{
	if(action != InputAction::RELEASE)
	{
		if(code == KeyCode::KEY_W)
		{
			activeCamera->getCameraComponent()->moveOrigin(1);
		}
		if(code == KeyCode::KEY_S)
		{
			activeCamera->getCameraComponent()->moveOrigin(-1);
		}
	}
}

static bool mouseDown = false;

void SceneView::mouseMoveCallback(double xPos, double yPos) 
{
	static double prevXPos = 0.0f, prevYPos = 0.0f;
	double deltaX = prevXPos - xPos;
	double deltaY = prevYPos - yPos;
	prevXPos = xPos;
	prevYPos = yPos;
	if(mouseDown)
	{
		activeCamera->getCameraComponent()->mouseMove((float)deltaX, (float)deltaY);
	}
}

void SceneView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier) 
{
	if(button == MouseButton::MOUSE_BUTTON_1 && action != InputAction::RELEASE)
	{
		mouseDown = true;
	}
	else
	{
		mouseDown = false;
	}
}

void SceneView::scrollCallback(double, double yOffset) 
{
	activeCamera->getCameraComponent()->mouseScroll(yOffset);
}

void SceneView::fileCallback(int, const char**) 
{
	
}
