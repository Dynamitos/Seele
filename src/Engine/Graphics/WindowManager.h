#pragma once
#include "GraphicsResources.h"
#include "Window.h"
#include "Containers/Array.h"

namespace Seele
{
	class WindowManager
	{
	public:
		WindowManager(GraphicsInitializer initializer);
		~WindowManager();
	private:
		Array<PWindow> windows;
	};
}