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
			CmdBufferBase(PGraphics graphics, VkCommandPool cmdPool);
			virtual ~CmdBufferBase();
			inline VkCommandBuffer getHandle()
			{
				return handle;
			}
			void reset();
			VkViewport currentViewport;
			VkRect2D currentScissor;
		protected:
			PGraphics graphics;
			VkCommandBuffer handle;
			VkCommandPool owner;
		};
		DEFINE_REF(CmdBufferBase);

		DECLARE_REF(SecondaryCmdBuffer);
		class CmdBuffer : public CmdBufferBase
		{
		public:
			CmdBuffer(PGraphics graphics, VkCommandPool cmdPool);
			virtual ~CmdBuffer();
			void begin();
			void end();
			void beginRenderPass(PRenderPass renderPass, PFramebuffer framebuffer);
			void endRenderPass();
			void executeCommands(Array<PSecondaryCmdBuffer> secondaryCommands);
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
			PRenderPass renderPass;
			PFramebuffer framebuffer;
			uint32 subpassIndex;
			State state;
			friend class SecondaryCmdBuffer;
			friend class CommandBufferManager;
		};
		DEFINE_REF(CmdBuffer);

		class SecondaryCmdBuffer : public CmdBufferBase
		{
		public:
			SecondaryCmdBuffer(PGraphics graphics, VkCommandPool cmdPool);
			virtual ~SecondaryCmdBuffer();
			void begin(PCmdBuffer parent);
			void end();
		private:
		};
		DEFINE_REF(SecondaryCmdBuffer);

		class CommandBufferManager
		{
		public:
			CommandBufferManager(PGraphics graphics, PQueue queue);
			virtual ~CommandBufferManager();
			PCmdBuffer getCommands();
			PSecondaryCmdBuffer createSecondaryCmdBuffer();
			void submitCommands(PSemaphore signalSemaphore = nullptr);
			void waitForCommands(PCmdBuffer cmdBuffer, float timeToWait = 1.0f);
		private:
			PGraphics graphics;
			VkCommandPool commandPool;
			PQueue queue;
			uint32 queueFamilyIndex;
			PCmdBuffer activeCmdBuffer;
			Array<PCmdBuffer> allocatedBuffers;
		};
		DEFINE_REF(CommandBufferManager);
	}
}