#include "Graphics/Graphics.h"
#include "VulkanGraphicsResources.h"

namespace Seele
{
	namespace Vulkan
	{
		DECLARE_REF(Allocator);
		class Graphics : public Gfx::Graphics
		{
		public:
			Graphics();
			virtual ~Graphics();
			VkDevice getDevice() const;
			VkPhysicalDevice getPhysicalDevice() const;

			// Inherited via Graphics
			virtual void init(GraphicsInitializer initializer) override;
			virtual void beginFrame(void* windowHandle) override;
			virtual void endFrame(void* windowHandle) override;
			virtual void* createWindow(const WindowCreateInfo& createInfo) override;
		protected:
			Array<const char*> getRequiredExtensions();
			void initInstance(GraphicsInitializer initInfo);
			void setupDebugCallback();
			void pickPhysicalDevice();
			void createDevice();

			VkDevice handle;
			VkPhysicalDevice physicalDevice;
			VkInstance instance;
			VkDebugReportCallbackEXT callback;
			Array<PViewport> viewports;
			PAllocator allocator;
			friend class Window;
		};
		DEFINE_REF(Graphics);
	}
}