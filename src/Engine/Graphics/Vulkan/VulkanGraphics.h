#pragma once
#include "Graphics/Graphics.h"
#include "VulkanGraphicsResources.h"

namespace Seele
{
	namespace Vulkan
	{
		DECLARE_REF(Allocator);
		DECLARE_REF(CommandBufferManager);
		DECLARE_REF(Queue);
		class Graphics : public Gfx::Graphics
		{
		public:
			Graphics();
			virtual ~Graphics();
			VkDevice getDevice() const;
			VkPhysicalDevice getPhysicalDevice() const;

			PCommandBufferManager getGraphicsCommands();
			PCommandBufferManager getComputeCommands();
			PCommandBufferManager getTransferCommands();
			PCommandBufferManager getDedicatedTransferCommands();

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
			void createDevice(GraphicsInitializer initInfo);

			VkDevice handle;
			VkPhysicalDevice physicalDevice;
			VkInstance instance;
			
			WQueue graphicsQueue;
			WQueue computeQueue;
			WQueue transferQueue;
			WQueue dedicatedTransferQueue;
			QueueFamilyMapping queueMapping;
			PCommandBufferManager graphicsCommands;
			PCommandBufferManager computeCommands;
			PCommandBufferManager transferCommands;
			PCommandBufferManager dedicatedTransferCommands;
			VkPhysicalDeviceProperties props;
			VkPhysicalDeviceFeatures features;
			VkDebugReportCallbackEXT callback;
			Array<PViewport> viewports;
			PAllocator allocator;

			friend class Window;
		};
		DEFINE_REF(Graphics);
	}
}