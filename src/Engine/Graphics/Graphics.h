#pragma once
#include "MinimalEngine.h"
#include "GraphicsResources.h"
#include "Containers/Array.h"

namespace Seele {
	class Window;
	class Graphics
	{
	public:
		Graphics();
		~Graphics();
		virtual void init(GraphicsInitializer initializer) = 0;
		virtual void beginFrame(void* windowHandle) = 0;
		virtual void endFrame(void* windowHandle) = 0;
		virtual void* createWindow(const WindowCreateInfo& createInfo) = 0;
	protected:
		friend class Window;
	};
	DEFINE_REF(Graphics);
}