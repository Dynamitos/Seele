#pragma once
#include "WindowManager.h"
#include "Scene/Scene.h"
namespace Seele
{
class RenderCore
{
public:
	RenderCore();
	~RenderCore();
	void init();
	void renderLoop();
	void shutdown();

private:
	PScene scene;
	PWindowManager windowManager;
};
} // namespace Seele