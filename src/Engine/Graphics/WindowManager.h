#pragma once
#include "GraphicsResources.h"
#include "Window.h"
#include "Containers/Array.h"

namespace Seele
{
	class WindowManager
	{
	public:
		WindowManager();
		~WindowManager();
		void addWindow(const WindowCreateInfo& createInfo);
		void beginFrame();
		void endFrame();
	private:
		Array<PWindow> windows;
		PGraphics graphics;
	};
	DEFINE_REF(WindowManager);
}