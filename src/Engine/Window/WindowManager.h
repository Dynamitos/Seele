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
	PWindow addWindow(Gfx::PGraphics graphics, const WindowCreateInfo &createInfo);
	void notifyWindowClosed(PWindow window);
	inline bool isActive() const
	{
		return windows.size();
	}

private:
	Array<OWindow> windows;
};
DEFINE_REF(WindowManager)
} // namespace Seele