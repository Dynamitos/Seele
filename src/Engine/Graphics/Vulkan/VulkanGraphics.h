#include "Graphics/Graphics.h"
#include "VulkanGraphicsResources.h"

namespace Seele
{
	DECLARE_REF(VulkanAllocator);
	class VulkanGraphics : public Graphics
	{
	public:
		VulkanGraphics();
		virtual ~VulkanGraphics();
		VkDevice getDevice() const;
		VkPhysicalDevice getPhysicalDevice() const;

		// Inherited via Graphics
		virtual void init(GraphicsInitializer initializer) override;
		virtual void beginFrame(void* windowHandle) override;
		virtual void endFrame(void* windowHandle) override;
		virtual void* createWindow(const WindowCreateInfo& createInfo) override;
	protected:
		Array<const char*> getRequiredExtensions();
		void setupDebugCallback();

		VkDevice handle;
		VkPhysicalDevice physicalDevice;
		VkInstance instance;
		VkDebugReportCallbackEXT callback;
		PVulkanAllocator allocator;
		friend class Window;
	};
	DEFINE_REF(VulkanGraphics);
}