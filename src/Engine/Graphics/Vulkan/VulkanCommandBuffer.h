#pragma once
#include "VulkanGraphicsResources.h"
#include "VulkanQueue.h"

namespace Seele
{
	namespace Vulkan
	{
		DECLARE_REF(RenderPass);
		DECLARE_REF(Framebuffer);
		class CmdBufferBase : public Gfx::RenderCommandBase
		{
		public:
			CmdBufferBase(WGraphics graphics, VkCommandPool cmdPool);
			virtual ~CmdBufferBase();
			inline VkCommandBuffer getHandle()
			{
				return handle;
			}
			void reset();
			VkViewport currentViewport;
			VkRect2D currentScissor;
		protected:
			WGraphics graphics;
			VkCommandBuffer handle;
			VkCommandPool owner;
		};
		DEFINE_REF(CmdBufferBase);

		DECLARE_REF(SecondaryCmdBuffer);
		class CmdBuffer : public CmdBufferBase
		{
		public:
			CmdBuffer(WGraphics graphics, VkCommandPool cmdPool);
			virtual ~CmdBuffer();
			void begin();
			void end();
			void beginRenderPass(WRenderPass renderPass, WFramebuffer framebuffer);
			void endRenderPass();
			void executeCommands(Array<WSecondaryCmdBuffer> secondaryCommands);
			void addWaitSemaphore(VkPipelineStageFlags stages, PSemaphore waitSemaphore);
			enum State
			{
				ReadyBegin,
				InsideBegin,
				RenderPassActive,
				Ended,
				Submitted,
			};

		private:
			WRenderPass renderPass;
			WFramebuffer framebuffer;
			uint32 subpassIndex;
			State state;
			friend class SecondaryCmdBuffer;
		};
		DEFINE_REF(CmdBuffer);

		class SecondaryCmdBuffer : public CmdBufferBase
		{
		public:
			SecondaryCmdBuffer(WGraphics graphics, VkCommandPool cmdPool);
			virtual ~SecondaryCmdBuffer();
			void begin(WCmdBuffer parent);
			void end();
		private:
		};
		DEFINE_REF(SecondaryCmdBuffer);

		class CommandBufferManager
		{
		public:
			CommandBufferManager(WGraphics graphics, WQueue queue);
			virtual ~CommandBufferManager();
			PCmdBuffer getCommands();
			PSecondaryCmdBuffer createSecondaryCmdBuffer();
			void submitCommands(PSemaphore signalSemaphore = nullptr);
			void waitForCommands(PCmdBuffer cmdBuffer, float timeToWait = 1.0f);
		private:
			WGraphics graphics;
			VkCommandPool commandPool;
			WQueue queue;
			uint32 queueFamilyIndex;
			PCmdBuffer activeCmdBuffer;
			Array<PCmdBuffer> allocatedBuffers;
		};
		DEFINE_REF(CommandBufferManager);
	}
}