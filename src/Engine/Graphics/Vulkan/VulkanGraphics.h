#include "Graphics/Graphics.h"

namespace Seele
{
	class VulkanGraphics : public Graphics
	{
	public:
		// Inherited via Graphics
		virtual void init(GraphicsInitializer initializer) override;
		virtual void beginFrame(void* windowHandle) override;
		virtual void endFrame(void* windowHandle) override;
		virtual void* createWindow(const WindowCreateInfo& createInfo) override;
	protected:
		VulkanGraphics();
		virtual ~VulkanGraphics();
		friend class Window;
	};
}