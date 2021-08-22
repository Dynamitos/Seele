#pragma once
#include "VulkanGraphicsResources.h"
#include "VulkanQueue.h"

namespace Seele
{
struct VertexInputStream;
namespace Vulkan
{
DECLARE_REF(RenderPass)
DECLARE_REF(Framebuffer)
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
DEFINE_REF(CmdBufferBase)

DECLARE_REF(RenderCommand)
DECLARE_REF(ComputeCommand)
DECLARE_REF(DescriptorSet)
DECLARE_REF(CommandBufferManager)
class CmdBuffer : public CmdBufferBase
{
public:
	CmdBuffer(PGraphics graphics, VkCommandPool cmdPool, PCommandBufferManager manager);
	virtual ~CmdBuffer();
	void begin();
	void end();
	void beginRenderPass(PRenderPass renderPass, PFramebuffer framebuffer);
	void endRenderPass();
	void executeCommands(const Array<Gfx::PRenderCommand>& secondaryCommands);
	void executeCommands(const Array<Gfx::PComputeCommand>& secondaryCommands);
	void addWaitSemaphore(VkPipelineStageFlags stages, PSemaphore waitSemaphore);
	void refreshFence();
	void waitForCommand(uint32 timeToWait = 1000000u);
	PFence getFence();
	PCommandBufferManager getManager();
	enum State
	{
		ReadyBegin,
		InsideBegin,
		RenderPassActive,
		Ended,
		Submitted,
	};

private:
	PCommandBufferManager manager;
	PRenderPass renderPass;
	PFramebuffer framebuffer;
	PFence fence;
	uint32 subpassIndex;
	State state;
	Array<PSemaphore> waitSemaphores;
	Array<VkPipelineStageFlags> waitFlags;
	Array<PRenderCommand> executingRenders;
	Array<PComputeCommand> executingComputes;
	Array<DescriptorSet*> boundDescriptors;
	friend class RenderCommand;
	friend class CommandBufferManager;
	friend class Queue;
};
DEFINE_REF(CmdBuffer)

DECLARE_REF(GraphicsPipeline)
DECLARE_REF(ComputePipeline)

class SecondaryCmdBuffer: public CmdBufferBase
{
public:
	SecondaryCmdBuffer(PGraphics graphics, VkCommandPool cmdPool);
	virtual ~SecondaryCmdBuffer();
	virtual void begin(PCmdBuffer parent) = 0;
	void end();
	void reset();
	bool ready;

protected:
	Array<DescriptorSet*> boundDescriptors;
	friend class CmdBuffer;
};
DEFINE_REF(SecondaryCmdBuffer);

class RenderCommand : public Gfx::RenderCommand, public SecondaryCmdBuffer
{
public:
	RenderCommand(PGraphics graphics, VkCommandPool cmdPool);
	virtual ~RenderCommand();
	virtual void begin(PCmdBuffer parent) override;
	virtual bool isReady() override;
	virtual void setViewport(Gfx::PViewport viewport) override;
	virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) override;
	virtual void bindDescriptor(Gfx::PDescriptorSet descriptorSet) override;
	virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets) override;
	virtual void bindVertexBuffer(const Array<VertexInputStream>& streams) override;
	virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) override;
	virtual void draw(const MeshBatchElement& data) override;

private:
	PGraphicsPipeline pipeline;
	friend class CmdBuffer;
};
DEFINE_REF(RenderCommand)

class ComputeCommand : public Gfx::ComputeCommand, public SecondaryCmdBuffer
{
public:
	ComputeCommand(PGraphics graphics, VkCommandPool cmdPool);
	virtual ~ComputeCommand();
	virtual void begin(PCmdBuffer parent) override;
	virtual bool isReady() override;
	virtual void bindPipeline(Gfx::PComputePipeline pipeline) override;
	virtual void bindDescriptor(Gfx::PDescriptorSet set) override;
	virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) override;
	virtual void dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) override;
private:
	PComputePipeline pipeline;
	friend class CmdBuffer;
};
DEFINE_REF(ComputeCommand)
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
	PRenderCommand createRenderCommand(const std::string& name);
	PComputeCommand createComputeCommand(const std::string& name);
	VkCommandPool getPoolHandle() const
	{
		return commandPool;
	}
	void submitCommands(PSemaphore signalSemaphore = nullptr);

private:
	PGraphics graphics;
	VkCommandPool commandPool;
	PQueue queue;
	uint32 queueFamilyIndex;
	PCmdBuffer activeCmdBuffer;
	std::mutex allocatedBufferLock;
	Array<PCmdBuffer> allocatedBuffers;
	std::mutex allocatedRenderLock;
	std::mutex allocatedComputeLock;
	Array<PRenderCommand> allocatedRenderCommands;
	Array<PComputeCommand> allocatedComputeCommands;
};
DEFINE_REF(CommandBufferManager)
} // namespace Vulkan
} // namespace Seele