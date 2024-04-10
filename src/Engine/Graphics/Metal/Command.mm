#include "Command.h"
#include "Graphics/Graphics.h"
#include "Graphics/Metal/Resources.h"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Window.h"

using namespace Seele;
using namespace Seele::Metal;

Command::Command(PGraphics graphics, PCommandQueue owner)
    : graphics(graphics), owner(owner), completed(new Event(graphics)),
      cmdBuffer(owner->getHandle()->commandBuffer()), renderEncoder(nullptr) {}

Command::~Command() { cmdBuffer->release(); }

void Command::beginRenderPass(PRenderPass renderPass) {
  renderEncoder =
      cmdBuffer->parallelRenderCommandEncoder(renderPass->getDescriptor());
}

void Command::endRenderPass() { renderEncoder->endEncoding(); }

void Command::present(MTL::Drawable *drawable) {
  cmdBuffer->presentDrawable(drawable);
}

void Command::end(PEvent signal) {
  if (signal != nullptr) {
    cmdBuffer->encodeSignalEvent(signal->getHandle(), 1);
  }
  cmdBuffer->encodeSignalEvent(completed->getHandle(), 1);
  cmdBuffer->commit();
}

void Command::executeCommands(Array<Gfx::ORenderCommand> commands) {
  for (auto& command : commands) {
    auto cmd = Gfx::PRenderCommand(command).cast<RenderCommand>();
    cmd->end();
  }
}

void Command::executeCommands(Array<Gfx::OComputeCommand> commands) {
  for (auto& command : commands) {
    auto cmd = Gfx::PComputeCommand(command).cast<ComputeCommand>();
    cmd->end();
  }
}

void Command::waitDeviceIdle() {
  cmdBuffer->waitUntilCompleted();
}

void Command::signalEvent(PEvent event) {
  cmdBuffer->encodeSignalEvent(event->getHandle(), 1);
}

void Command::waitForEvent(PEvent event) {
  cmdBuffer->encodeWait(event->getHandle(), 1);
}

RenderCommand::RenderCommand(MTL::RenderCommandEncoder *encoder)
    : encoder(encoder) {}

RenderCommand::~RenderCommand() { encoder->release(); }

void RenderCommand::end() { encoder->endEncoding(); }

void RenderCommand::setViewport(Gfx::PViewport viewport) {
  MTL::Viewport vp = viewport.cast<Viewport>()->getHandle();
  MTL::ScissorRect rect = {
      .x = static_cast<NS::UInteger>(vp.originX),
      .y = static_cast<NS::UInteger>(vp.originY),
      .width = static_cast<NS::UInteger>(vp.width),
      .height = static_cast<NS::UInteger>(vp.height),
  };
  encoder->setViewport(vp);
  encoder->setScissorRect(rect);
}

void RenderCommand::bindPipeline(Gfx::PGraphicsPipeline pipeline) {
  encoder->setRenderPipelineState(
      pipeline.cast<GraphicsPipeline>()->getState());
}

void RenderCommand::bindDescriptor(Gfx::PDescriptorSet descriptorSet) {
  encoder->set ? ? ? ? ?
}

void RenderCommand::bindDescriptor(
    const Array<Gfx::PDescriptorSet> &descriptorSets) {
  encoder->set ? ? ? ? ?
}

void RenderCommand::bindVertexBuffer(const Array<Gfx::PVertexBuffer> &buffers) {
  encoder->setVertexBuffers();
}

void RenderCommand::bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) {
    encoder->setObjectBuffer(??, NS::UInteger offset, NS::UInteger index)
}

void RenderCommand::pushConstants(Gfx::PPipelineLayout layout,
                                  Gfx::SeShaderStageFlags stage, uint32 offset,
                                  uint32 size, const void *data) {
    ? ? ?
}

void RenderCommand::draw(uint32 vertexCount, uint32 instanceCount,
                         int32 firstVertex, uint32 firstInstance) {
    encoder->drawPrimitives(???, firstVertex, vertexCount, instanceCount, firstInstance);
}

void RenderCommand::drawIndexed(uint32 indexCount, uint32 instanceCount,
                                int32 firstIndex, uint32 vertexOffset,
                                uint32 firstInstance) {
    encoder->drawIndexedPrimitives(???, indexCount, indexbuffer->getType(), indexBuffer->getBuffer(), firstIndex, instanceCount, vertexOffset, firstInstance);
}

void RenderCommand::drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ){
    encoder->drawMeshThreads(MTL::Size threadsPerGrid,
                             MTL::Size threadsPerObjectThreadgroup,
                             MTL::Size threadsPerMeshThreadgroup)}

ComputeCommand::ComputeCommand(MTL::ComputeCommandEncoder *encoder)
    : encoder(encoder) {}

ComputeCommand::~ComputeCommand() { encoder->release(); }

void ComputeCommand::end() { encoder->endEncoding(); }

void ComputeCommand::bindPipeline(Gfx::PComputePipeline pipeline) {
    encoder->setComputePipelineState(pipeline.cast<ComputePipeline>());
}

void ComputeCommand::bindDescriptor(Gfx::PDescriptorSet set) {
    encoder->set ? ? ?
}

void ComputeCommand::bindDescriptor(const Array<Gfx::PDescriptorSet> &sets) {
    encoder->set ? ? ?
}

void ComputeCommand::pushConstants(Gfx::PPipelineLayout layout,
                                   Gfx::SeShaderStageFlags stage, uint32 offset,
                                   uint32 size, const void *data) {
    ? ? ?
}

void ComputeCommand::dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) {
    encoder->dispatchThreadgroups(???);
}

CommandQueue::CommandQueue(PGraphics graphics) : graphics(graphics) {
    queue = graphics->getDevice()->newCommandQueue();
    activeCommand = new Command(graphics, this);
}

CommandQueue::~CommandQueue() { queue->release(); }

ORenderCommand CommandQueue::getRenderCommand(const std::string &name) {
    return new RenderCommand(activeCommand->createRenderEncoder());
}

OComputeCommand CommandQueue::getComputeCommand(const std::string &name) {
    return new ComputeCommand(activeCommand->createComputeEncoder());
}

void CommandQueue::submitCommands(PEvent signalSemaphore) {
    activeCommand->end(signalSemaphore);
    MTL::Event* prevCmdEvent = activeCommand->getCompletedEvent();
    activeCommand = new Command(graphics, this);
    activeCommand->waitForEvent(prevCmdEvent);
}
