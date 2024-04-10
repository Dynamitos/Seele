#include "Command.h"
#include "Metal/MTLComputeCommandEncoder.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Window.h"

using namespace Seele;
using namespace Seele::Metal;

Command::Command(PGraphics graphics, PCommandQueue owner)
    : graphics(graphics)
    , owner(owner)
    , cmdBuffer(owner->getHandle()->commandBuffer())
    , renderEncoder(nullptr)
{
}

Command::~Command()
{
    cmdBuffer->release();
}

void Command::beginRenderPass(PRenderPass renderPass)
{
    renderEncoder = cmdBuffer->parallelRenderCommandEncoder(renderPass->getDescriptor());
}

void Command::endRenderPass()
{
    renderEncoder->endEncoding();
}

void Command::executeCommands(const Array<Gfx::PRenderCommand>& commands)
{
    for(auto command : commands)
    {
        auto cmd = command.cast<RenderCommand>();
        cmd->end();
    }
}

void Command::executeCommands(const Array<Gfx::PComputeCommand>& commands)
{
    for(auto command : commands)
    {
        auto cmd = command.cast<ComputeCommand>();
        cmd->end();
    }
}

RenderCommand::RenderCommand(MTL::RenderCommandEncoder* encoder)
    : encoder(encoder)
{}

RenderCommand::~RenderCommand()
{
    encoder->release();
}

void RenderCommand::end()
{
    encoder->endEncoding();
}

void RenderCommand::setViewport(Gfx::PViewport viewport)
{
    encoder->setViewport(viewport.cast<Viewport>()->getHandle());
}

void RenderCommand::bindPipeline(Gfx::PGraphicsPipeline pipeline)
{
    encoder->setRenderPipelineState(pipeline.cast<GraphicsPipeline>()->getState());
}

void RenderCommand::bindDescriptor(Gfx::PDescriptorSet descriptorSet)
{
    encoder->set?????
}

void RenderCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& descriptorSets)
{
    encoder->set?????

}

void RenderCommand::bindVertexBuffer(const Array<Gfx::PVertexBuffer>& buffers)
{
    encoder->setVertexBuffers();
}

void RenderCommand::bindIndexBuffer(Gfx::PIndexBuffer indexBuffer)
{
    encoder->setObjectBuffer(??, NS::UInteger offset, NS::UInteger index)
}

void RenderCommand::pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data)
{
???
}

void RenderCommand::draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance)
{
    encoder->drawPrimitives(???, firstVertex, vertexCount, instanceCount, firstInstance);
}

void RenderCommand::drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance)
{
    encoder->drawIndexedPrimitives(???, indexCount, indexbuffer->getType(), indexBuffer->getBuffer(), firstIndex, instanceCount, vertexOffset, firstInstance);
}

void RenderCommand::drawMesh(uint32 groupX, uint32 groupY, uint32 groupZ)
{
    encoder->drawMeshThreads(MTL::Size threadsPerGrid, MTL::Size threadsPerObjectThreadgroup, MTL::Size threadsPerMeshThreadgroup)
}

ComputeCommand::ComputeCommand(MTL::ComputeCommandEncoder* encoder)
    : encoder(encoder)
{}

ComputeCommand::~ComputeCommand()
{
    encoder->release();
}

void ComputeCommand::end()
{
    encoder->endEncoding();
}

void ComputeCommand::bindPipeline(Gfx::PComputePipeline pipeline)
{
    encoder->setComputePipelineState(pipeline);
}

void ComputeCommand::bindDescriptor(Gfx::PDescriptorSet set)
{
    encoder->set???
}

void ComputeCommand::bindDescriptor(const Array<Gfx::PDescriptorSet>& sets)
{
    encoder->set???
}

void ComputeCommand::pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data)
{
    ???
}

void ComputeCommand::dispatch(uint32 threadX, uint32 threadY, uint32 threadZ)
{
    encoder->dispatchThreadgroups(???);
}

CommandQueue::CommandQueue(PGraphics graphics)
    : graphics(graphics)
{
    queue = graphics->getDevice()->newCommandQueue();
}

CommandQueue::~CommandQueue()
{
    queue->release();
}

PRenderCommand CommandQueue::getRenderCommand(const std::string& name)
{
}

PComputeCommand CommandQueue::getComputeCommand(const std::string& name)
{
    
}
