#pragma once
#include "Buffer.h"
#include "Graphics/Command.h"
#include "Queue.h"
#include "Resources.h"
#include <thread>

namespace Seele {
namespace Vulkan {
DECLARE_REF(RenderPass)
DECLARE_REF(Framebuffer)
DECLARE_REF(RenderCommand)
DECLARE_REF(ComputeCommand)
DECLARE_REF(DescriptorSet)
DECLARE_REF(CommandPool)
class Command {
  public:
    Command(PGraphics graphics, PCommandPool pool);
    ~Command();
    constexpr VkCommandBuffer getHandle() { return handle; }
    void reset();
    void begin();
    void end();
    void beginRenderPass(PRenderPass renderPass, PFramebuffer framebuffer);
    void endRenderPass();
    void executeCommands(Array<Gfx::ORenderCommand> secondaryCommands);
    void executeCommands(Array<Gfx::OComputeCommand> secondaryCommands);
    void waitForSemaphore(VkPipelineStageFlags stages, PSemaphore waitSemaphore);
    void bindResource(PCommandBoundResource resource);
    void checkFence();
    void waitForCommand(uint32 timeToWait = 1000000u);
    void setPipelineStatisticsFlags(VkQueryPipelineStatisticFlags flags);
    PFence getFence();
    PCommandPool getPool();
    enum State {
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
    PRenderPass boundRenderPass;
    PFramebuffer boundFramebuffer;
    Array<PSemaphore> waitSemaphores;
    Array<VkPipelineStageFlags> waitFlags;
    Array<ORenderCommand> executingRenders;
    Array<OComputeCommand> executingComputes;
    Array<PDescriptorSet> boundResources;
    VkQueryPipelineStatisticFlags statisticsFlags;
    friend class RenderCommand;
    friend class CommandPool;
    friend class Queue;
};
DEFINE_REF(Command)

DECLARE_REF(GraphicsPipeline)
DECLARE_REF(ComputePipeline)
DECLARE_REF(RayTracingPipeline)
class RenderCommand : public Gfx::RenderCommand {
  public:
    RenderCommand(PGraphics graphics, VkCommandPool cmdPool);
    virtual ~RenderCommand();
    constexpr VkCommandBuffer getHandle() { return handle; }
    void begin(PRenderPass renderPass, PFramebuffer framebuffer, VkQueryPipelineStatisticFlags pipelineFlags);
    void end();
    void reset();
    bool isReady();
    virtual void setViewport(Gfx::PViewport viewport) override;
    virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) override;
    virtual void bindPipeline(Gfx::PRayTracingPipeline pipeline) override;
    virtual void bindDescriptor(Gfx::PDescriptorSet descriptorSet, Array<uint32> dynamicOffsets) override;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets, Array<uint32> dynamicOffsets) override;
    virtual void bindVertexBuffer(const Array<Gfx::PVertexBuffer>& buffers) override;
    virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) override;
    virtual void pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
    virtual void draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) override;
    virtual void drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override;
    virtual void drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ) override;
    virtual void drawMeshIndirect(Gfx::PShaderBuffer buffer, uint64 offset, uint32 drawCount, uint32 stride) override;
    virtual void traceRays(uint32 width, uint32 height, uint32 depth) override;

  private:
    PGraphicsPipeline pipeline;
    PRayTracingPipeline rtPipeline;
    bool ready;
    Array<PCommandBoundResource> boundResources;
    VkViewport currentViewport;
    VkRect2D currentScissor;
    PGraphics graphics;
    std::thread::id threadId;
    VkCommandBuffer handle;
    VkCommandPool owner;
    friend class Command;
};
DEFINE_REF(RenderCommand)

class ComputeCommand : public Gfx::ComputeCommand {
  public:
    ComputeCommand(PGraphics graphics, VkCommandPool cmdPool);
    virtual ~ComputeCommand();
    inline VkCommandBuffer getHandle() { return handle; }
    void begin(VkQueryPipelineStatisticFlags pipelineFlags);
    void end();
    void reset();
    bool isReady();
    virtual void bindPipeline(Gfx::PComputePipeline pipeline) override;
    virtual void bindDescriptor(Gfx::PDescriptorSet set, Array<uint32> dynamicOffsets) override;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets, Array<uint32> dynamicOffsets) override;
    virtual void pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
    virtual void dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) override;

  private:
    PComputePipeline pipeline;
    bool ready;
    Array<PCommandBoundResource> boundResources;
    VkViewport currentViewport;
    VkRect2D currentScissor;
    PGraphics graphics;
    std::thread::id threadId;
    VkCommandBuffer handle;
    VkCommandPool owner;
    friend class Command;
};
DEFINE_REF(ComputeCommand)
class CommandPool {
  public:
    CommandPool(PGraphics graphics, PQueue queue);
    virtual ~CommandPool();
    constexpr PQueue getQueue() const { return queue; }
    PCommand getCommands();
    void cacheCommands(Array<ORenderCommand> commands);
    void cacheCommands(Array<OComputeCommand> commands);
    ORenderCommand createRenderCommand(const std::string& name);
    OComputeCommand createComputeCommand(const std::string& name);
    constexpr VkCommandPool getHandle() const { return commandPool; }
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