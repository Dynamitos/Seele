#pragma once
#include "GraphicsResources.h"
#include "Graphics.h"
#include "Containers/Array.h"

namespace Seele
{
class WindowManager
{
public:
	WindowManager();
	~WindowManager();
	void addWindow(const WindowCreateInfo &createInfo);
	void beginFrame();
	void endFrame();
	inline bool isActive() const
	{
		return windows.size();
	}

private:
	Array<Gfx::PWindow> windows;
	Gfx::PGraphics graphics;
};
DEFINE_REF(WindowManager);
} // namespace Seele