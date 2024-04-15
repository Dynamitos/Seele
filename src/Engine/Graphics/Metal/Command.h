#pragma once
#include "Graphics/Command.h"
#include "Buffer.h"
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
class Command
{
public:
    Command(PGraphics graphics, MTL::CommandBuffer* cmdBuffer);
    ~Command();
    void beginRenderPass(PRenderPass renderPass);
    void endRenderPass();
    void present(MTL::Drawable* drawable);
    void end(PEvent signal);
    void executeCommands(Array<Gfx::ORenderCommand> commands);
    void executeCommands(Array<Gfx::OComputeCommand> commands);
    void waitDeviceIdle();
    void signalEvent(PEvent event);
    void waitForEvent(PEvent event);
    MTL::RenderCommandEncoder* createRenderEncoder() { return renderEncoder->renderCommandEncoder(); }
    MTL::ComputeCommandEncoder* createComputeEncoder() { return cmdBuffer->computeCommandEncoder(); }
    PEvent getCompletedEvent() const { return completed; }
    constexpr MTL::CommandBuffer* getHandle() const
    {
      return cmdBuffer;
    }
private:
    PGraphics graphics;
    OEvent completed;
    MTL::CommandBuffer* cmdBuffer;
    MTL::ParallelRenderCommandEncoder* renderEncoder;
};
DEFINE_REF(Command)
class RenderCommand : public Gfx::RenderCommand
{
public:
    RenderCommand(MTL::RenderCommandEncoder* encoder);
    virtual ~RenderCommand();
    void end();
	virtual void setViewport(Gfx::PViewport viewport) override;
	virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) override;
	virtual void bindDescriptor(Gfx::PDescriptorSet descriptorSet) override;
	virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets) override;
	virtual void bindVertexBuffer(const Array<Gfx::PVertexBuffer>& buffers) override;
	virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) override;
	virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
	virtual void draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) override;
	virtual void drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) override; 
	virtual void drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ) override;
private:
    MTL::RenderCommandEncoder* encoder;
    PIndexBuffer boundIndexBuffer;
    PGraphicsPipeline boundPipeline;
};
DEFINE_REF(RenderCommand)
class ComputeCommand : public Gfx::ComputeCommand
{
public:
    ComputeCommand(MTL::ComputeCommandEncoder* encoder);
    virtual ~ComputeCommand();
    void end();
	virtual void bindPipeline(Gfx::PComputePipeline pipeline) override;
	virtual void bindDescriptor(Gfx::PDescriptorSet set) override;
	virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) override;
	virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) override;
	virtual void dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) override;
private:
    MTL::ComputeCommandEncoder* encoder;
};
DEFINE_REF(ComputeCommand)
class CommandQueue
{
public:
    CommandQueue(PGraphics graphics);
    ~CommandQueue();
    constexpr MTL::CommandQueue* getHandle()
    {
        return queue;
    }
    PCommand getCommands() {return activeCommand;} //TODO
    ORenderCommand getRenderCommand(const std::string& name);
    OComputeCommand getComputeCommand(const std::string& name);
    void submitCommands(PEvent signal = nullptr);
private:
    PGraphics graphics;
    MTL::CommandQueue* queue;
    OCommand activeCommand;
    Array<OCommand> pendingCommands;
};
class IOCommandQueue
{
public:
  IOCommandQueue(PGraphics graphics);
  ~IOCommandQueue();
  constexpr MTL::IOCommandQueue* getHandle()
  {
    return queue;
  }
  PCommand getCommands() {return activeCommand;} // TODO
  void submitCommands(PEvent signal = nullptr);
private:
  PGraphics graphics;
  MTL::IOCommandQueue* queue;
  OCommand activeCommand;
};
DEFINE_REF(IOCommandQueue)
}
}