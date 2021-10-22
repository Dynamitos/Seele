#pragma once
#include "Graphics/GraphicsResources.h"
#include "Graphics/Graphics.h"
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
	static Gfx::PGraphics getGraphics()
	{
		return graphics;
	}
	inline bool isActive() const
	{
		return windows.size();
	}

private:
	Array<PWindow> windows;
	static Gfx::PGraphics graphics;
};
DEFINE_REF(WindowManager)
} // namespace Seele