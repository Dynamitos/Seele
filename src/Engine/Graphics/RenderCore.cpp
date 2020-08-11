#include "RenderCore.h"
#include "SceneView.h"

Seele::RenderCore::RenderCore()
{
	windowManager = new WindowManager();
}

Seele::RenderCore::~RenderCore()
{
}

void Seele::RenderCore::init()
{
	WindowCreateInfo mainWindowInfo;
	mainWindowInfo.title = "SeeleEngine";
	mainWindowInfo.width = 1280;
	mainWindowInfo.height = 720;
	mainWindowInfo.bFullscreen = false;
	mainWindowInfo.numSamples = 1;
	mainWindowInfo.pixelFormat = Gfx::SE_FORMAT_R8G8B8_UNORM;
	auto window = windowManager->addWindow(mainWindowInfo);
	ViewportCreateInfo sceneViewInfo;
	sceneViewInfo.sizeX = 1280;
	sceneViewInfo.sizeY = 720;
	sceneViewInfo.offsetX = 0;
	sceneViewInfo.offsetY = 0;
	PSceneView sceneView = new SceneView(windowManager->getGraphics(), window, sceneViewInfo);
	window->addView(sceneView);
}

void Seele::RenderCore::renderLoop()
{
	while (windowManager->isActive())
	{
		windowManager->beginFrame();
		windowManager->endFrame();
	}
}

void Seele::RenderCore::shutdown()
{
}
