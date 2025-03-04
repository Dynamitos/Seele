#include "Command.h"
#include "Buffer.h"
#include "Containers/Array.h"
#include "Descriptor.h"
#include "Enums.h"
#include "Foundation/NSString.hpp"
#include "Graphics/Command.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Resources.h"
#include "Metal/MTLArgumentEncoder.hpp"
#include "Metal/MTLCaptureManager.hpp"
#include "Metal/MTLCommandEncoder.hpp"
#include "Metal/MTLLibrary.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Pipeline.h"
#include "Resources.h"
#include "Window.h"
#include <iostream>

using namespace Seele;
using namespace Seele::Metal;

Command::Command(PGraphics graphics, MTL::CommandBuffer* cmdBuffer)
    : graphics(graphics), completed(new Event(graphics)), cmdBuffer(cmdBuffer) {}

Command::~Command() {
    assert(parallelEncoder == nullptr);
    if(blitEncoder != nullptr) {
        blitEncoder->endEncoding();
        blitEncoder->release();
    }
    cmdBuffer->release();
}

void Command::beginRenderPass(PRenderPass renderPass) {
    if (blitEncoder) {
        blitEncoder->endEncoding();
        blitEncoder->release();
        blitEncoder = nullptr;
    }
    renderPass->updateRenderPass();
    parallelEncoder = cmdBuffer->parallelRenderCommandEncoder(renderPass->getDescriptor());
    parallelEncoder->setLabel(NS::String::string(renderPass->getName().c_str(), NS::ASCIIStringEncoding));
}

void Command::endRenderPass() {
    parallelEncoder->endEncoding();
    parallelEncoder->release();
    parallelEncoder = nullptr;
}

void Command::present(MTL::Drawable* drawable) { cmdBuffer->presentDrawable(drawable); }

void Command::end(PEvent signal) {
    assert(!parallelEncoder);
    if (blitEncoder) {
        blitEncoder->endEncoding();
        blitEncoder->release();
        blitEncoder = nullptr;
    }
    if (signal != nullptr) {
        cmdBuffer->encodeSignalEvent(signal->getHandle(), 1);
    }
    cmdBuffer->encodeSignalEvent(completed->getHandle(), 1);
    cmdBuffer->commit();
}

void Command::waitDeviceIdle() { cmdBuffer->waitUntilCompleted(); }

void Command::signalEvent(PEvent event) { cmdBuffer->encodeSignalEvent(event->getHandle(), 1); }

void Command::waitForEvent(PEvent event) { cmdBuffer->encodeWait(event->getHandle(), 1); }

RenderCommand::RenderCommand(MTL::RenderCommandEncoder* encoder, const std::string& name) : encoder(encoder), name(name) {}

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
    boundPipeline = pipeline.cast<GraphicsPipeline>();
    encoder->setRenderPipelineState(boundPipeline->getHandle());
    encoder->setDepthStencilState(boundPipeline->depth);
}

void RenderCommand::bindPipeline(Gfx::PRayTracingPipeline pipeline) {}

void RenderCommand::bindDescriptor(Gfx::PDescriptorSet descriptorSet) {
    auto metalSet = descriptorSet.cast<DescriptorSet>();
    metalSet->bind();
    uint32 descriptorIndex = boundPipeline->getPipelineLayout()->findParameter(metalSet->getLayout()->getName());
    
    auto createEncoder = [&metalSet, descriptorIndex](MTL::Function* function, const Array<uint32>& usedSets) {
        if(metalSet->encoder == nullptr) {
            if (metalSet->isPlainDescriptor()) {
                metalSet->encoder = metalSet->createEncoder();
            } else if (function != nullptr && usedSets.contains(descriptorIndex)) {
                metalSet->encoder = function->newArgumentEncoder(descriptorIndex);
            }
        }
    };
    createEncoder(boundPipeline->taskFunction, boundPipeline->taskSets);
    createEncoder(boundPipeline->meshFunction, boundPipeline->meshSets);
    createEncoder(boundPipeline->vertexFunction, boundPipeline->vertexSets);
    createEncoder(boundPipeline->fragmentFunction, boundPipeline->fragmentSets);
    if(metalSet->argumentBuffer == nullptr) {
        metalSet->argumentBuffer = metalSet->graphics->getDevice()->newBuffer(metalSet->encoder->encodedLength(), MTL::ResourceOptionCPUCacheModeDefault);
        metalSet->encoder->setArgumentBuffer(metalSet->argumentBuffer, 0);
    }
    for (const auto& write : metalSet->uniformWrites) {
        write.apply(metalSet->encoder);
    }
    for (const auto& write : metalSet->bufferWrites) {
        write.apply(metalSet->encoder);
        encoder->useResource(write.buffer, write.access);
    }
    for (const auto& write : metalSet->samplerWrites) {
        write.apply(metalSet->encoder);
    }
    for (const auto& write : metalSet->textureWrites) {
        write.apply(metalSet->encoder);
        encoder->useResource(write.texture, write.access);
    }
    for (const auto& write : metalSet->accelerationWrites) {
        write.apply(metalSet->encoder);
        encoder->useResource(write.accelerationStructure, write.access);
    }
    encoder->useResource(metalSet->argumentBuffer, MTL::ResourceUsageRead);
    encoder->setObjectBuffer(metalSet->argumentBuffer, 0, descriptorIndex);
    encoder->setMeshBuffer(metalSet->argumentBuffer, 0, descriptorIndex);
    encoder->setVertexBuffer(metalSet->argumentBuffer, 0, descriptorIndex);
    encoder->setFragmentBuffer(metalSet->argumentBuffer, 0, descriptorIndex);
}

void RenderCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets) {
    for (auto set : descriptorSets) {
        bindDescriptor(set);
    }
}
void RenderCommand::bindVertexBuffer(const Array<Gfx::PVertexBuffer>& buffers) {
    uint32 i = 0;
    for (auto buffer : buffers) {
        encoder->setVertexBuffer(buffer.cast<VertexBuffer>()->getHandle(), 0, buffer.cast<VertexBuffer>()->getVertexSize(), i++);
    }
}

void RenderCommand::bindIndexBuffer(Gfx::PIndexBuffer gfxIndexBuffer) { boundIndexBuffer = gfxIndexBuffer.cast<IndexBuffer>(); }

void RenderCommand::pushConstants(Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) {
    uint pushIndex = boundPipeline->getPipelineLayout()->findParameter("pOffsets");
    if (stage & Gfx::SE_SHADER_STAGE_VERTEX_BIT) {
        encoder->setVertexBytes(data, size, pushIndex); // TODO: hardcoded
    }
    if (stage & Gfx::SE_SHADER_STAGE_FRAGMENT_BIT) {
        encoder->setFragmentBytes(data, size, pushIndex); // TODO: hardcoded
    }
    if (stage & Gfx::SE_SHADER_STAGE_TASK_BIT_EXT) {
        encoder->setObjectBytes(data, size, pushIndex); // TODO: hardcoded
    }
    if (stage & Gfx::SE_SHADER_STAGE_MESH_BIT_EXT) {
        encoder->setMeshBytes(data, size, pushIndex); // TODO: hardcoded
    }
}

void RenderCommand::draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) {
    encoder->drawPrimitives(boundPipeline->getPrimitive(), firstVertex, vertexCount, instanceCount, firstInstance);
}

void RenderCommand::drawIndirect(Gfx::PShaderBuffer buffer, uint64 offset, uint32 drawCount, uint32 stride) {
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, buffer.cast<ShaderBuffer>()->getHandle(), offset);
}

void RenderCommand::drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) {
    encoder->drawIndexedPrimitives(boundPipeline->getPrimitive(), indexCount, cast(boundIndexBuffer->getIndexType()),
                                   boundIndexBuffer->getHandle(), firstIndex, instanceCount, vertexOffset, firstInstance);
}
void RenderCommand::drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ) {
    encoder->drawMeshThreadgroups(MTL::Size(groupX, groupY, groupZ), MTL::Size(128, 1, 1), MTL::Size(32, 1, 1));
}

void RenderCommand::drawMeshIndirect(Gfx::PShaderBuffer buffer, uint64 offset, uint32 drawCount, uint32 stride) {
    // encoder->drawMeshThreadgroups()
}

void RenderCommand::traceRays(uint32 width, uint32 height, uint32 depth) {}

ComputeCommand::ComputeCommand(MTL::CommandBuffer* cmdBuffer, const std::string& name)
    : commandBuffer(cmdBuffer), encoder(cmdBuffer->computeCommandEncoder()), name(name) {}

ComputeCommand::~ComputeCommand() { encoder->release(); commandBuffer->release(); }

void ComputeCommand::end() {
    encoder->endEncoding();
    commandBuffer->commit();
}

void ComputeCommand::bindPipeline(Gfx::PComputePipeline pipeline) {
    boundPipeline = pipeline.cast<ComputePipeline>();
    encoder->setComputePipelineState(boundPipeline->getHandle());
}

void ComputeCommand::bindDescriptor(Gfx::PDescriptorSet set) {
    auto metalSet = set.cast<DescriptorSet>();
    metalSet->bind();
    uint32 descriptorIndex = boundPipeline->getPipelineLayout()->findParameter(metalSet->getLayout()->getName());
    if(metalSet->encoder == nullptr) {
        if (metalSet->isPlainDescriptor()) {
            metalSet->encoder = metalSet->createEncoder();
        } else {
            metalSet->encoder = boundPipeline->computeFunction->newArgumentEncoder(descriptorIndex);
        }
        metalSet->argumentBuffer = metalSet->graphics->getDevice()->newBuffer(metalSet->encoder->encodedLength(), MTL::ResourceOptionCPUCacheModeDefault);
        metalSet->encoder->setArgumentBuffer(metalSet->argumentBuffer, 0);
    }
    for (const auto& write : metalSet->uniformWrites) {
        write.apply(metalSet->encoder);
    }
    for (const auto& write : metalSet->bufferWrites) {
        write.apply(metalSet->encoder);
        encoder->useResource(write.buffer, write.access);
    }
    for (const auto& write : metalSet->samplerWrites) {
        write.apply(metalSet->encoder);
    }
    for (const auto& write : metalSet->textureWrites) {
        write.apply(metalSet->encoder);
        encoder->useResource(write.texture, write.access);
    }
    for (const auto& write : metalSet->accelerationWrites) {
        write.apply(metalSet->encoder);
        encoder->useResource(write.accelerationStructure, write.access);
    }
    encoder->useResource(metalSet->argumentBuffer, MTL::ResourceUsageRead);
    encoder->setBuffer(metalSet->argumentBuffer, 0, descriptorIndex);
}

void ComputeCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) {
    for (auto& set : sets) {
        bindDescriptor(set);
    }
}

void ComputeCommand::pushConstants(Gfx::SeShaderStageFlags, uint32 offset, uint32 size, const void* data) {
    uint pushIndex = boundPipeline->getPipelineLayout()->findParameter("pMipParam");
    encoder->setBytes(data, size, pushIndex); // TODO: hardcoded
}

void ComputeCommand::dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) {
    // TODO
    encoder->dispatchThreadgroups(MTL::Size(threadX, threadY, threadZ), MTL::Size(32, 32, 1));
}

void ComputeCommand::dispatchIndirect(Gfx::PShaderBuffer buffer, uint32 offset) {
    encoder->dispatchThreadgroups(buffer.cast<ShaderBuffer>()->getHandle(), offset, MTL::Size(32, 32, 1));
}

CommandQueue::CommandQueue(PGraphics graphics) : graphics(graphics) {
    queue = graphics->getDevice()->newCommandQueue();
    MTL::CommandBufferDescriptor* descriptor = MTL::CommandBufferDescriptor::alloc()->init();
    descriptor->setErrorOptions(MTL::CommandBufferErrorOptionEncoderExecutionStatus);
    activeCommand = new Command(graphics, queue->commandBuffer(descriptor));
    descriptor->release();
}

CommandQueue::~CommandQueue() { queue->release(); }

ORenderCommand CommandQueue::getRenderCommand(const std::string& name) {
    return new RenderCommand(activeCommand->createRenderEncoder(), name);
}

OComputeCommand CommandQueue::getComputeCommand(const std::string& name) { return new ComputeCommand(queue->commandBuffer(), name); }

void CommandQueue::executeCommands(Array<Gfx::ORenderCommand> commands) {
    for (auto& command : commands) {
        auto metalCmd = Gfx::PRenderCommand(command).cast<RenderCommand>();
        metalCmd->end();
    }
}

void CommandQueue::executeCommands(Array<Gfx::OComputeCommand> commands) {
    for (auto& command : commands) {
        auto metalCmd = Gfx::PComputeCommand(command).cast<ComputeCommand>();
        metalCmd->end();
    }
}

void CommandQueue::submitCommands(PEvent signalSemaphore) {
    activeCommand->getHandle()->addCompletedHandler(MTL::CommandBufferHandler([&](MTL::CommandBuffer* cmdBuffer) {
        for (auto it = pendingCommands.begin(); it != pendingCommands.end(); it++) {
            if ((*it)->getHandle() == cmdBuffer) {
                readyCommands.add(std::move(*it));
                pendingCommands.erase(it);
                return;
            }
        }
    }));
    activeCommand->end(signalSemaphore);
    activeCommand->waitDeviceIdle();
    PEvent prevCmdEvent = activeCommand->getCompletedEvent();
    pendingCommands.add(std::move(activeCommand));
    if (!readyCommands.empty()) {
        activeCommand = std::move(readyCommands.front());
        readyCommands.removeAt(0);
        activeCommand->waitForEvent(prevCmdEvent);
    } else {
        MTL::CommandBufferDescriptor* descriptor = MTL::CommandBufferDescriptor::alloc()->init();
        descriptor->setErrorOptions(MTL::CommandBufferErrorOptionEncoderExecutionStatus);
        activeCommand = new Command(graphics, queue->commandBuffer(descriptor));
        activeCommand->waitForEvent(prevCmdEvent);
        descriptor->release();
    }
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
