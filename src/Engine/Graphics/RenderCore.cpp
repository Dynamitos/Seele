#include "RenderCore.h"

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
	windowManager->addWindow(mainWindowInfo);
}

void Seele::RenderCore::renderLoop()
{
	while (true)
	{
		windowManager->beginFrame();
		windowManager->endFrame();
	}
}

void Seele::RenderCore::shutdown()
{
}
