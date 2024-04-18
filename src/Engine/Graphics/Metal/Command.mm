#include "Command.h"
#include "Buffer.h"
#include "Containers/Array.h"
#include "Descriptor.h"
#include "Enums.h"
#include "Graphics/Command.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Resources.h"
#include "Pipeline.h"
#include "Resources.h"
#include "Window.h"
#include <Metal/Metal.h>

using namespace Seele;
using namespace Seele::Metal;

Command::Command(PGraphics graphics, MTL::CommandBuffer* cmdBuffer)
    : graphics(graphics), completed(new Event(graphics)), cmdBuffer(cmdBuffer) {}

Command::~Command() {}

void Command::beginRenderPass(PRenderPass renderPass) {
  if (blitEncoder) {
    blitEncoder->endEncoding();
    blitEncoder = nullptr;
  }
  renderPass->updateRenderPass();
  parallelEncoder = cmdBuffer->parallelRenderCommandEncoder(renderPass->getDescriptor());
}

void Command::endRenderPass() {
  parallelEncoder->endEncoding();
  parallelEncoder = nullptr;
}

void Command::present(MTL::Drawable* drawable) { cmdBuffer->presentDrawable(drawable); }

void Command::end(PEvent signal) {
  assert(!parallelEncoder);
  blitEncoder->endEncoding();
  if (signal != nullptr) {
    cmdBuffer->encodeSignalEvent(signal->getHandle(), 1);
  }
  cmdBuffer->encodeSignalEvent(completed->getHandle(), 1);
  cmdBuffer->commit();
}

void Command::waitDeviceIdle() { cmdBuffer->waitUntilCompleted(); }

void Command::signalEvent(PEvent event) { cmdBuffer->encodeSignalEvent(event->getHandle(), 1); }

void Command::waitForEvent(PEvent event) { cmdBuffer->encodeWait(event->getHandle(), 1); }

RenderCommand::RenderCommand(MTL::RenderCommandEncoder* encoder, const std::string& name)
    : encoder(encoder), name(name) {}

RenderCommand::~RenderCommand() {}

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
  boundPipeline = pipeline.cast<GraphicsPipeline>();
  encoder->setRenderPipelineState(boundPipeline->getHandle());
}

void RenderCommand::bindDescriptor(Gfx::PDescriptorSet descriptorSet) {
  encoder->setVertexBuffer(descriptorSet.cast<DescriptorSet>()->getBuffer(), 0, descriptorSet->getSetIndex());
  encoder->setFragmentBuffer(descriptorSet.cast<DescriptorSet>()->getBuffer(), 0, descriptorSet->getSetIndex());
  encoder->setMeshBuffer(descriptorSet.cast<DescriptorSet>()->getBuffer(), 0, descriptorSet->getSetIndex());
  encoder->setObjectBuffer(descriptorSet.cast<DescriptorSet>()->getBuffer(), 0, descriptorSet->getSetIndex());
}

void RenderCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets) {
  for (auto set : descriptorSets) {
    bindDescriptor(set);
  }
}
void RenderCommand::bindVertexBuffer(const Array<Gfx::PVertexBuffer>& buffers) {
  uint32 i = 0;
  for (auto buffer : buffers) {
    encoder->setVertexBuffer(buffer.cast<VertexBuffer>()->getHandle(), 0, METAL_VERTEXBUFFER_OFFSET + i++);
  }
}

void RenderCommand::bindIndexBuffer(Gfx::PIndexBuffer gfxIndexBuffer) {
  boundIndexBuffer = gfxIndexBuffer.cast<IndexBuffer>();
}

void RenderCommand::pushConstants(Gfx::PPipelineLayout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size,
                                  const void* data) {
  if (stage & Gfx::SE_SHADER_STAGE_VERTEX_BIT) {
    encoder->setVertexBytes((char*)data + offset, size, 0);
  }
  if (stage & Gfx::SE_SHADER_STAGE_FRAGMENT_BIT) {
    encoder->setFragmentBytes((char*)data + offset, size, 0);
  }
}

void RenderCommand::draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) {
  encoder->drawPrimitives(boundPipeline->getPrimitive(), firstVertex, vertexCount, instanceCount, firstInstance);
}

void RenderCommand::drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset,
                                uint32 firstInstance) {
  encoder->drawIndexedPrimitives(boundPipeline->getPrimitive(), indexCount, cast(boundIndexBuffer->getIndexType()),
                                 boundIndexBuffer->getHandle(), firstIndex, instanceCount, vertexOffset, firstInstance);
}

void RenderCommand::drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ) {
  // TODO:
  encoder->drawMeshThreadgroups(MTL::Size(groupX, groupY, groupZ), MTL::Size(128, 128, 128), MTL::Size(32, 32, 32));
}

ComputeCommand::ComputeCommand(MTL::CommandBuffer* cmdBuffer, const std::string& name)
    : commandBuffer(cmdBuffer), encoder(cmdBuffer->computeCommandEncoder()), name(name) {}

ComputeCommand::~ComputeCommand() {}

void ComputeCommand::end() {
  encoder->endEncoding();
  commandBuffer->commit();
}

void ComputeCommand::bindPipeline(Gfx::PComputePipeline pipeline) {
  encoder->setComputePipelineState(pipeline.cast<ComputePipeline>()->getHandle());
}

void ComputeCommand::bindDescriptor(Gfx::PDescriptorSet set) {
  auto metalSet = set.cast<DescriptorSet>();
  metalSet->bind();
  encoder->setBuffer(metalSet->getBuffer(), 0, set->getSetIndex());
}

void ComputeCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) {
  for (auto& set : sets) {
    bindDescriptor(set);
  }
}

void ComputeCommand::pushConstants(Gfx::PPipelineLayout, Gfx::SeShaderStageFlags, uint32 offset, uint32 size,
                                   const void* data) {
  encoder->setBytes((char*)data + offset, size, 0);
}

void ComputeCommand::dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) {
  // TODO
  encoder->dispatchThreadgroups(MTL::Size(threadX, threadY, threadZ), MTL::Size(32, 32, 32));
}

CommandQueue::CommandQueue(PGraphics graphics) : graphics(graphics) {
  queue = graphics->getDevice()->newCommandQueue();
  MTL::CommandBufferDescriptor* descriptor = MTL::CommandBufferDescriptor::alloc()->init();
  descriptor->setErrorOptions(MTL::CommandBufferErrorOptionEncoderExecutionStatus);
  activeCommand = new Command(graphics, queue->commandBuffer(descriptor));
}

CommandQueue::~CommandQueue() { queue->release(); }

ORenderCommand CommandQueue::getRenderCommand(const std::string& name) {
  return new RenderCommand(activeCommand->createRenderEncoder(), name);
}

OComputeCommand CommandQueue::getComputeCommand(const std::string& name) {
  return new ComputeCommand(queue->commandBuffer(), name);
}

void CommandQueue::executeCommands(Array<Gfx::ORenderCommand> commands) {
  for (auto& command : commands) {
    auto metalCmd = Gfx::PRenderCommand(command).cast<RenderCommand>();
    metalCmd->end();
  }
}

void CommandQueue::executeCommands(Array<Gfx::OComputeCommand> commands) {
  submitCommands();
  for (auto& command : commands) {
    auto metalCmd = Gfx::PComputeCommand(command).cast<ComputeCommand>();
    metalCmd->end();
  }
}

void CommandQueue::submitCommands(PEvent signalSemaphore) {
  activeCommand->getHandle()->addCompletedHandler(MTL::CommandBufferHandler([&](MTL::CommandBuffer* cmdBuffer) {
    for (auto it = pendingCommands.begin(); it != pendingCommands.end(); it++) {
      if ((*it)->getHandle() == cmdBuffer) {
        pendingCommands.remove(it);
        return;
      }
    }
  }));
  activeCommand->end(signalSemaphore);
  activeCommand->waitDeviceIdle();
  PEvent prevCmdEvent = activeCommand->getCompletedEvent();
  pendingCommands.add(std::move(activeCommand));
  MTL::CommandBufferDescriptor* descriptor = MTL::CommandBufferDescriptor::alloc()->init();
  descriptor->setErrorOptions(MTL::CommandBufferErrorOptionEncoderExecutionStatus);
  activeCommand = new Command(graphics, queue->commandBuffer(descriptor));
  activeCommand->waitForEvent(prevCmdEvent);
}

IOCommandQueue::IOCommandQueue(PGraphics graphics) : graphics(graphics) {
  MTL::IOCommandQueueDescriptor* desc = MTL::IOCommandQueueDescriptor::alloc()->init();
  desc->setType(MTL::IOCommandQueueTypeConcurrent);
  desc->setPriority(MTL::IOPriorityNormal);
  queue = graphics->getDevice()->newIOCommandQueue(desc, nullptr);
  // activeCommand = new Command(graphics, queue->commandBuffer());
  desc->release();
}

IOCommandQueue::~IOCommandQueue() { queue->release(); }

void IOCommandQueue::submitCommands(PEvent signalSemaphore) {
  // TODO: scratch buffer
  activeCommand->end(signalSemaphore);
  PEvent prevCmdEvent = activeCommand->getCompletedEvent();
  // activeCommand = new Command(graphics, queue->commandBuffer());
  activeCommand->waitForEvent(prevCmdEvent);
}
