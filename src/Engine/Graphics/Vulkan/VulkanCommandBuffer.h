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
DECLARE_REF(RenderCommand)
DECLARE_REF(ComputeCommand)
DECLARE_REF(DescriptorSet)
DECLARE_REF(CommandBufferManager)
class CmdBuffer
{
public:
	CmdBuffer(PGraphics graphics, VkCommandPool cmdPool, PCommandBufferManager manager);
	virtual ~CmdBuffer();
	inline VkCommandBuffer getHandle()
	{
		return handle;
	}
	void reset();
	void begin();
	void end();
	void beginRenderPass(PRenderPass renderPass, PFramebuffer framebuffer);
	void endRenderPass();
	void executeCommands(const Array<Gfx::PRenderCommand>& secondaryCommands);
	void executeCommands(const Array<Gfx::PComputeCommand>& secondaryCommands);
	void addWaitSemaphore(VkPipelineStageFlags stages, PSemaphore waitSemaphore);
	void refreshFence();
	void waitForCommand(uint32 timeToWait = 1000000u);
	Fence* operator co_await()
	{
		return fence.getHandle();
	}
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
	PGraphics graphics;
	PCommandBufferManager manager;
	PFence fence;
	State state;
	VkViewport currentViewport;
	VkRect2D currentScissor;
	std::mutex handleLock;
	VkCommandBuffer handle;
	VkCommandPool owner;
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
class RenderCommand : public Gfx::RenderCommand
{
public:
	RenderCommand(PGraphics graphics, VkCommandPool cmdPool);
	virtual ~RenderCommand();
	inline VkCommandBuffer getHandle()
	{
		return handle;
	}
	void begin(PRenderPass renderPass, PFramebuffer framebuffer);
	void end();
	void reset();
	virtual bool isReady() override;
	virtual void setViewport(Gfx::PViewport viewport) override;
	virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) override;
	virtual void bindDescriptor(Gfx::PDescriptorSet descriptorSet) override;
	virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets) override;
	virtual void bindVertexBuffer(const Array<VertexInputStream>& streams) override;
	virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) override;
	virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
	virtual void draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) override;
	virtual void dispatch(uint32 groupX, uint32 groupY, uint32 groupZ) override;
private:
	PGraphicsPipeline pipeline;
	bool ready;
	Array<DescriptorSet*> boundDescriptors;
	VkViewport currentViewport;
	VkRect2D currentScissor;
	PGraphics graphics;
	std::thread::id threadId;
	VkCommandBuffer handle;
	VkCommandPool owner;
	friend class CmdBuffer;
};
DEFINE_REF(RenderCommand)

class ComputeCommand : public Gfx::ComputeCommand
{
public:
	ComputeCommand(PGraphics graphics, VkCommandPool cmdPool);
	virtual ~ComputeCommand();
	inline VkCommandBuffer getHandle()
	{
		return handle;
	}
	void begin(PCmdBuffer parent);
	void end();
	void reset();
	virtual bool isReady() override;
	virtual void bindPipeline(Gfx::PComputePipeline pipeline) override;
	virtual void bindDescriptor(Gfx::PDescriptorSet set) override;
	virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) override;
	virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
	virtual void dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) override;
private:
	PComputePipeline pipeline;
	bool ready;
	Array<DescriptorSet*> boundDescriptors;
	VkViewport currentViewport;
	VkRect2D currentScissor;
	PGraphics graphics;
	std::thread::id threadId;
	VkCommandBuffer handle;
	VkCommandPool owner;
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
	PRenderCommand createRenderCommand(PRenderPass renderPass, PFramebuffer framebuffer, const std::string& name);
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