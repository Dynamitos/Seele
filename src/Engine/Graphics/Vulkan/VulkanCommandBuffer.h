#pragma once
#include "VulkanGraphicsResources.h"
#include "VulkanQueue.h"

namespace Seele
{
struct VertexInputStream;
namespace Vulkan
{
DECLARE_REF(RenderPass);
DECLARE_REF(Framebuffer);
class CmdBufferBase
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
	void refreshFence();
	PFence getFence();
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
	PFence fence;
	uint32 subpassIndex;
	State state;
	Array<PSemaphore> waitSemaphores;
	Array<VkPipelineStageFlags> waitFlags;
	friend class SecondaryCmdBuffer;
	friend class CommandBufferManager;
	friend class Queue;
};
DEFINE_REF(CmdBuffer);

DECLARE_REF(GraphicsPipeline);
class SecondaryCmdBuffer : public CmdBufferBase, public Gfx::RenderCommand
{
public:
	SecondaryCmdBuffer(PGraphics graphics, VkCommandPool cmdPool);
	virtual ~SecondaryCmdBuffer();
	void begin(PCmdBuffer parent);
	void end();
	virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) override;
	virtual void bindDescriptor(Gfx::PDescriptorSet descriptorSet) override;
	virtual void bindVertexBuffer(const Array<VertexInputStream>& streams) override;
	virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) override;
	virtual void draw(const MeshBatchElement& data) override;

private:
	PGraphicsPipeline pipeline;
};
DEFINE_REF(SecondaryCmdBuffer);

class CommandBufferManager
{
public:
	CommandBufferManager(PGraphics graphics, PQueue queue);
	virtual ~CommandBufferManager();
	inline PQueue getQueue() const
	{
		return queue;
	}
	PCmdBuffer getCommands();
	PSecondaryCmdBuffer createSecondaryCmdBuffer();
	void submitCommands(PSemaphore signalSemaphore = nullptr);
	void waitForCommands(PCmdBuffer cmdBuffer, uint32 timeToWait = 1000000u);

private:
	PGraphics graphics;
	VkCommandPool commandPool;
	PQueue queue;
	uint32 queueFamilyIndex;
	PCmdBuffer activeCmdBuffer;
	Array<PCmdBuffer> allocatedBuffers;
};
DEFINE_REF(CommandBufferManager);
} // namespace Vulkan
} // namespace Seele