#include "SceneView.h"
#include "Scene/Scene.h"
#include "Window.h"
#include "Scene/Actor/CameraActor.h"
#include "Scene/Components/CameraComponent.h"
#include "Graphics/RenderPass/DepthPrepass.h"
#include "Graphics/RenderPass/LightCullingPass.h"
#include "Graphics/RenderPass/BasePass.h"

using namespace Seele;

Seele::SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo)
	: View(graphics, owner, createInfo)
{
	scene = new Scene(graphics);
	activeCamera = new CameraActor();
	scene->addActor(activeCamera);
	renderGraph = new RenderGraph();
	renderGraph->addRenderPass(new DepthPrepass(renderGraph, scene, graphics, viewport, activeCamera));
	renderGraph->addRenderPass(new LightCullingPass(renderGraph, scene, graphics, viewport, activeCamera));
	renderGraph->addRenderPass(new BasePass(renderGraph, scene, graphics, viewport, activeCamera));
	renderGraph->setup();
}

Seele::SceneView::~SceneView()
{
}

void SceneView::beginFrame() 
{
	View::beginFrame();
	scene->tick(Gfx::currentFrameDelta);
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
