#pragma once
#include "Buffer.h"
#include "Graphics/Command.h"
#include "Metal/MTLArgumentEncoder.hpp"
#include "Metal/MTLBlitCommandEncoder.hpp"
#include "Metal/MTLCaptureManager.hpp"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLComputeCommandEncoder.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "MinimalEngine.h"
#include "RenderPass.h"
#include "Resources.h"

namespace Seele {
namespace Metal {
DECLARE_REF(CommandQueue)
DECLARE_REF(ComputeCommand)
DECLARE_REF(RenderCommand)
DECLARE_REF(Graphics)
DECLARE_REF(IndexBuffer)
DECLARE_REF(GraphicsPipeline)
DECLARE_REF(ComputePipeline)
DECLARE_REF(DescriptorSet)
class Command {
  public:
    Command(PGraphics graphics, MTL::CommandBuffer* cmdBuffer);
    ~Command();
    void begin();
    void end(PEvent signal = nullptr);
    void beginRenderPass(PRenderPass renderPass);
    void endRenderPass();
    void present(MTL::Drawable* drawable);
    void waitDeviceIdle();
    void reset();
    void signalEvent(PEvent event);
    void waitForEvent(PEvent event);
    void bindResource(PCommandBoundResource res) {
        res->bind();
        boundResources.add(res);
    }
    enum State {
        Init,
        Begin,
        RenderPass,
        Submit,
    };
    MTL::RenderCommandEncoder* createRenderEncoder() { return parallelEncoder->renderCommandEncoder(); }
    MTL::BlitCommandEncoder* getBlitEncoder() {
        assert(!parallelEncoder);
        if (blitEncoder == nullptr) {
            blitEncoder = cmdBuffer->blitCommandEncoder();
        }
        return blitEncoder;
    }
    PEvent getCompletedEvent() const { return completed; }
    constexpr MTL::CommandBuffer* getHandle() const { return cmdBuffer; }

  private:
    PGraphics graphics;
    OEvent completed;
    State state;
    MTL::CommandBuffer* cmdBuffer;
    MTL::ParallelRenderCommandEncoder* parallelEncoder = nullptr;
    MTL::BlitCommandEncoder* blitEncoder = nullptr;
    Array<PCommandBoundResource> boundResources;
    friend class CommandQueue;
};
DEFINE_REF(Command)
class RenderCommand : public Gfx::RenderCommand {
  public:
    RenderCommand(MTL::RenderCommandEncoder* encode, const std::string& name);
    virtual ~RenderCommand();
    void end();
    virtual void setViewport(Gfx::PViewport viewport) override;
    virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) override;
    virtual void bindPipeline(Gfx::PRayTracingPipeline pipeline) override;
    virtual void bindDescriptor(Gfx::PDescriptorSet descriptorSet) override;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets) override;
    virtual void bindVertexBuffer(const Array<Gfx::PVertexBuffer>& buffers) override;
    virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) override;
    virtual void pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
    virtual void draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) override;
    virtual void drawIndirect(Gfx::PShaderBuffer buffer, uint64 offset, uint32 drawCount, uint32 stride) override;
    virtual void drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override;
    virtual void drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ) override;
    virtual void drawMeshIndirect(Gfx::PShaderBuffer buffer, uint64 offset, uint32 drawCount, uint32 stride) override;
    virtual void traceRays(uint32 width, uint32 height, uint32 depth) override;

  private:
    PGraphicsPipeline boundPipeline;
    PIndexBuffer boundIndexBuffer;
    Array<PCommandBoundResource> boundResources;
    MTL::RenderCommandEncoder* encoder;
    std::string name;
    friend class CommandQueue;
};
DEFINE_REF(RenderCommand)
class ComputeCommand : public Gfx::ComputeCommand {
  public:
    ComputeCommand(MTL::CommandBuffer* cmdBuffer, const std::string& name);
    virtual ~ComputeCommand();
    void end();
    virtual void bindPipeline(Gfx::PComputePipeline pipeline) override;
    virtual void bindDescriptor(Gfx::PDescriptorSet set) override;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) override;
    virtual void pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
    virtual void dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) override;
    virtual void dispatchIndirect(Gfx::PShaderBuffer buffer, uint32 offset) override;

  private:
    PComputePipeline boundPipeline;
    MTL::CommandBuffer* commandBuffer;
    Array<PCommandBoundResource> boundResources;
    MTL::ComputeCommandEncoder* encoder;
    std::string name;
    friend class CommandQueue;
};
DEFINE_REF(ComputeCommand)
class CommandQueue {
  public:
    CommandQueue(PGraphics graphics);
    ~CommandQueue();
    constexpr MTL::CommandQueue* getHandle() { return queue; }
    PCommand getCommands() { return activeCommand; }
    ORenderCommand getRenderCommand(const std::string& name);
    OComputeCommand getComputeCommand(const std::string& name);
    void executeCommands(Array<Gfx::ORenderCommand> commands);
    void executeCommands(Array<Gfx::OComputeCommand> commands);
    void submitCommands(PEvent signal = nullptr);

  private:
    PGraphics graphics;
    MTL::CommandQueue* queue;
    OCommand activeCommand;
    Array<OCommand> pendingCommands;
    Array<OCommand> readyCommands;
};
class IOCommandQueue {
  public:
    IOCommandQueue(PGraphics graphics);
    ~IOCommandQueue();
    constexpr MTL::IOCommandQueue* getHandle() { return queue; }
    PCommand getCommands() { return activeCommand; } // TODO
    void submitCommands(PEvent signal = nullptr);

  private:
    PGraphics graphics;
    MTL::IOCommandQueue* queue;
    OCommand activeCommand;
};
DEFINE_REF(IOCommandQueue)
} // namespace Metal
} // namespace Seele
