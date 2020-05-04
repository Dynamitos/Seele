#pragma once
#include "GraphicsResources.h"
#include "Graphics.h"
#include "Containers/Array.h"
#include "Window.h"

namespace Seele
{
class WindowManager
{
public:
	WindowManager();
	~WindowManager();
	PWindow addWindow(const WindowCreateInfo &createInfo);
	void beginFrame();
	void endFrame();
	Gfx::PGraphics getGraphics()
	{
		return graphics;
	}
	inline bool isActive() const
	{
		return windows.size();
	}

private:
	Array<PWindow> windows;
	Gfx::PGraphics graphics;
};
DEFINE_REF(WindowManager);
} // namespace Seele