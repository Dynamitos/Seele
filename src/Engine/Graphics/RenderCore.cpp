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
}

void Seele::RenderCore::renderLoop()
{
	while (windowManager->isActive())
	{
		windowManager->beginFrame();
		windowManager->render();
		windowManager->endFrame();
	}
}

void Seele::RenderCore::shutdown()
{
}
