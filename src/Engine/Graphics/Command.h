#pragma once
#include "Enums.h"
#include "Descriptor.h"

namespace Seele
{
namespace Gfx
{
DECLARE_REF(Viewport)
class RenderCommand
{
public:
    RenderCommand();
    virtual ~RenderCommand();
    virtual bool isReady() = 0;
    virtual void setViewport(Gfx::PViewport viewport) = 0;
    virtual void bindPipeline(Gfx::PGraphicsPipeline pipeline) = 0;
    virtual void bindDescriptor(Gfx::PDescriptorSet set) = 0;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) = 0;
    virtual void bindVertexBuffer(const Array<PVertexBuffer>& buffer) = 0;
    virtual void bindIndexBuffer(Gfx::PIndexBuffer indexBuffer) = 0;
    virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) = 0;
    virtual void draw(uint32 vertexCount, uint32 instanceCount, int32 firstVertex, uint32 firstInstance) = 0;
    virtual void drawIndexed(uint32 indexCount, uint32 instanceCount, int32 firstIndex, uint32 vertexOffset, uint32 firstInstance) = 0;
    virtual void dispatch(uint32 groupX, uint32 groupY, uint32 groupZ) = 0;
    std::string name;
};
DEFINE_REF(RenderCommand)
    class ComputeCommand
{
public:
    ComputeCommand();
    virtual ~ComputeCommand();
    virtual bool isReady() = 0;
    virtual void bindPipeline(Gfx::PComputePipeline pipeline) = 0;
    virtual void bindDescriptor(Gfx::PDescriptorSet set) = 0;
    virtual void bindDescriptor(const Array<Gfx::PDescriptorSet>& sets) = 0;
    virtual void pushConstants(Gfx::PPipelineLayout layout, Gfx::SeShaderStageFlags stage, uint32 offset, uint32 size, const void* data) = 0;
    virtual void dispatch(uint32 threadX, uint32 threadY, uint32 threadZ) = 0;
    std::string name;
};
DEFINE_REF(ComputeCommand)

}
}