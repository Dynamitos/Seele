#pragma once
#include "Window/WindowManager.h"
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

	PWindowManager getWindowManager() const { return windowManager; };
private:
	PScene scene;
	PWindowManager windowManager;
};
} // namespace Seele