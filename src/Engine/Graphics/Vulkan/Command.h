#pragma once
#include "Queue.h"
#include "Graphics/Command.h"
#include "Buffer.h"

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
DECLARE_REF(CommandPool)
class Command
{
public:
	Command(PGraphics graphics, VkCommandPool cmdPool, PCommandPool pool);
	virtual ~Command();
	constexpr VkCommandBuffer getHandle()
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
	void waitForSemaphore(VkPipelineStageFlags stages, PSemaphore waitSemaphore);
	void checkFence();
	void waitForCommand(uint32 timeToWait = 1000000u);
	PFence getFence();
	PCommandPool getPool();
	enum State
	{
		Init,
		Begin,
		RenderPass,
		End,
		Submit,
	};

private:
	PGraphics graphics;
	PCommandPool pool;
	OFence fence;
	OSemaphore signalSemaphore;
	State state;
	VkViewport currentViewport;
	VkRect2D currentScissor;
	VkCommandBuffer handle;
	VkCommandPool owner;
	PRenderPass boundRenderPass;
	PFramebuffer boundFramebuffer;
	Array<PSemaphore> waitSemaphores;
	Array<VkPipelineStageFlags> waitFlags;
	Array<PRenderCommand> executingRenders;
	Array<PComputeCommand> executingComputes;
	Array<PDescriptorSet> boundDescriptors;
	friend class RenderCommand;
	friend class CommandPool;
	friend class Queue;
};
DEFINE_REF(Command)

DECLARE_REF(GraphicsPipeline)
DECLARE_REF(ComputePipeline)
class RenderCommand : public Gfx::RenderCommand
{
public:
	RenderCommand(PGraphics graphics, VkCommandPool cmdPool);
	virtual ~RenderCommand();
	constexpr VkCommandBuffer getHandle()
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
	virtual void bindVertexBuffer(const Array<Gfx::PVertexBuffer>& buffers) override;
	virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) override;
	virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
	virtual void draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) override;
	virtual void drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override; 
	virtual void dispatch(uint32 groupX, uint32 groupY, uint32 groupZ) override;
private:
	PGraphicsPipeline pipeline;
	bool ready;
	Array<PDescriptorSet> boundDescriptors;
	VkViewport currentViewport;
	VkRect2D currentScissor;
	PGraphics graphics;
	std::thread::id threadId;
	VkCommandBuffer handle;
	VkCommandPool owner;
	friend class Command;
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
	void begin();
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
	friend class Command;
};
DEFINE_REF(ComputeCommand)
class CommandPool
{
public:
	CommandPool(PGraphics graphics, PQueue queue);
	virtual ~CommandPool();
	constexpr PQueue getQueue() const
	{
		return queue;
	}
	PCommand getCommands();
	PRenderCommand createRenderCommand(const std::string& name);
	PComputeCommand createComputeCommand(const std::string& name);
	constexpr VkCommandPool getHandle() const
	{
		return commandPool;
	}
	void submitCommands(PSemaphore signalSemaphore = nullptr);

private:
	PGraphics graphics;
	VkCommandPool commandPool;
	PQueue queue;
	uint32 queueFamilyIndex;
	PCommand command;
	Array<OCommand> allocatedBuffers;
	Array<ORenderCommand> allocatedRenderCommands;
	Array<OComputeCommand> allocatedComputeCommands;
};
DEFINE_REF(CommandPool)
} // namespace Vulkan
} // namespace Seele