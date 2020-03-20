#include "VulkanGraphicsResources.h"

namespace Seele
{
	namespace Vulkan
	{
		class CmdBufferBase : public RenderCommandBase
		{
		public:
		private:
			VkCommandBuffer handle;
		};
		DEFINE_REF(CmdBuffer);

		class CmdBuffer : public CmdBufferBase
		{

		};
		DEFINE_REF(CmdBuffer);

		class CommandBufferManager
		{
		public:
			CommandBufferManager(Vulkan::PGraphics graphics, PVulkanQueue queue);
			virtual ~CommandBufferManager();
			PCmdBuffer getCommands();
			PSecondaryCommandBuffer createSecondaryCmdBuffer();
			void submitCommands(PSemaphore signalSemaphore = nullptr);
			void waitForBuffer(PCommandbuffer cmdBuffer, float timeToWait = 1.0f);
		private:
			VkCommandPool commandPool;
			QueueType queueType;
			uint32 queueFamilyIndex;
			Array<Vulkan::PCmdBuffer> allocatedBuffers;
		};
		DEFINE_REF(VulkanCommandBufferManager);
	}
}