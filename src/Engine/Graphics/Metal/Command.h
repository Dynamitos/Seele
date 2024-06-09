#pragma once
#include "Buffer.h"
#include "Graphics/Command.h"
#include "Metal/MTLBlitCommandEncoder.hpp"
#include "Metal/MTLCaptureManager.hpp"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLComputeCommandEncoder.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
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
class Command {
  public:
    Command(PGraphics graphics, MTL::CommandBuffer* cmdBuffer);
    ~Command();
    void beginRenderPass(PRenderPass renderPass);
    void endRenderPass();
    void present(MTL::Drawable* drawable);
    void end(PEvent signal);
    void waitDeviceIdle();
    void signalEvent(PEvent event);
    void waitForEvent(PEvent event);
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
    MTL::CommandBuffer* cmdBuffer;
    MTL::ParallelRenderCommandEncoder* parallelEncoder = nullptr;
    MTL::BlitCommandEncoder* blitEncoder = nullptr;
};
DEFINE_REF(Command)
class RenderCommand : public Gfx::RenderCommand {
  public:
    RenderCommand(MTL::RenderCommandEncoder* encode, const std::string& name);
    virtual ~RenderCommand();
    void end();
    virtual void setViewport(Gfx::PViewport viewport) override;
    virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) override;
    virtual void bindDescriptor(Gfx::PDescriptorSet descriptorSet, Array<uint32> dynamicOffsets) override;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets, Array<uint32> dynamicOffsets) override;
    virtual void bindVertexBuffer(const Array<Gfx::PVertexBuffer>& buffers) override;
    virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) override;
    virtual void pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
    virtual void draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) override;
    virtual void drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override;
    virtual void drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ) override;
    virtual void drawMeshIndirect(Gfx::PShaderBuffer buffer, uint64 offset, uint32 drawCount, uint32 stride) override;

  private:
    PGraphicsPipeline boundPipeline;
    PIndexBuffer boundIndexBuffer;
    MTL::Buffer* argumentBuffer;
    MTL::RenderCommandEncoder* encoder;
    std::string name;
};
DEFINE_REF(RenderCommand)
class ComputeCommand : public Gfx::ComputeCommand {
  public:
    ComputeCommand(MTL::CommandBuffer* cmdBuffer, const std::string& name);
    virtual ~ComputeCommand();
    void end();
    virtual void bindPipeline(Gfx::PComputePipeline pipeline) override;
    virtual void bindDescriptor(Gfx::PDescriptorSet set, Array<uint32> dynamicOffsets) override;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets, Array<uint32> dynamicOffsets) override;
    virtual void pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
    virtual void dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) override;

  private:
    PComputePipeline boundPipeline;
    MTL::CommandBuffer* commandBuffer;
    MTL::ComputeCommandEncoder* encoder;
    MTL::Buffer* argumentBuffer;
    std::string name;
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
