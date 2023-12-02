#pragma once
#include "Containers/Array.h"
#include "Window.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Graphics)
class WindowManager
{
public:
	WindowManager();
	~WindowManager();
	OWindow addWindow(Gfx::PGraphics graphics, const WindowCreateInfo &createInfo);
	void render();
	void notifyWindowClosed(PWindow window);
	bool isActive() const
	{
		return windows.size();
	}

private:
	Array<PWindow> windows;
};
DEFINE_REF(WindowManager)
} // namespace Seele