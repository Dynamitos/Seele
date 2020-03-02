#include "Graphics/Graphics.h"

namespace Seele
{
	class VulkanGraphics : public Graphics
	{
	public:
		// Inherited via Graphics
		virtual void init(GraphicsInitializer initializer) override;
		virtual void beginFrame() override;
		virtual void endFrame() override;
		virtual void* createWindow(const WindowCreateInfo& createInfo) override;
	};
}