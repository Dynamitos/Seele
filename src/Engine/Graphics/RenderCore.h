#pragma once
#include "WindowManager.h"
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
	PWindowManager windowManager;
};
} // namespace Seele