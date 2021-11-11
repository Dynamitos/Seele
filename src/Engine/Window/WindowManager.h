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
	Job init();
	PWindow addWindow(const WindowCreateInfo &createInfo);
	void notifyWindowClosed(PWindow window);
	static Gfx::PGraphics getGraphics()
	{
		return graphics;
	}
	inline bool isActive() const
	{
		return windows.size();
	}
	void waitForCompletion()
	{
		std::unique_lock lock(windowsLock);
		windowsCV.wait(lock);
	}

private:
	Array<PWindow> windows;
	std::mutex windowsLock;
	std::condition_variable windowsCV;
	static Gfx::PGraphics graphics;
};
DEFINE_REF(WindowManager)
} // namespace Seele