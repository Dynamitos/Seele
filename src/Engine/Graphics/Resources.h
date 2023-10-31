#pragma once
#include "Math/Math.h"
#include "Enums.h"
#include "Containers/Array.h"
#include "Containers/List.h"
#include "Initializer.h"
#include "CRC.h"
#include <functional>


#ifndef ENABLE_VALIDATION
#define ENABLE_VALIDATION 0
#endif

namespace Seele
{
DECLARE_REF(Material)
namespace Gfx
{
DECLARE_REF(DescriptorSet)
DECLARE_REF(Graphics)
DECLARE_REF(VertexBuffer)
DECLARE_REF(IndexBuffer)
class SamplerState
{
public:
    virtual ~SamplerState()
    {
    }
};
DEFINE_REF(SamplerState)

struct QueueFamilyMapping
{
    uint32 graphicsFamily;
    uint32 computeFamily;
    uint32 transferFamily;
    uint32 dedicatedTransferFamily;
    uint32 getQueueTypeFamilyIndex(Gfx::QueueType type) const
    {
        switch (type)
        {
        case Gfx::QueueType::GRAPHICS:
            return graphicsFamily;
        case Gfx::QueueType::COMPUTE:
            return computeFamily;
        case Gfx::QueueType::TRANSFER:
            return transferFamily;
        case Gfx::QueueType::DEDICATED_TRANSFER:
            return dedicatedTransferFamily;
        default:
            return 0x7fff;
        }
    }
    bool needsTransfer(Gfx::QueueType src, Gfx::QueueType dst) const
    {
        uint32 srcIndex = getQueueTypeFamilyIndex(src);
        uint32 dstIndex = getQueueTypeFamilyIndex(dst);
        return srcIndex != dstIndex;
    }
};

class QueueOwnedResource
{
public:
    QueueOwnedResource(QueueFamilyMapping mapping, QueueType startQueueType);
    virtual ~QueueOwnedResource();

    //Preliminary checks to see if the barrier should be executed at all
    void transferOwnership(QueueType newOwner);
    void pipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess, SePipelineStageFlags dstStage);

protected:
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, 
        SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
    Gfx::QueueType currentOwner;
    QueueFamilyMapping mapping;
};
DEFINE_REF(QueueOwnedResource)

class VertexDeclaration
{
public:
    VertexDeclaration();
    ~VertexDeclaration();

    static PVertexDeclaration createDeclaration(PGraphics graphics, const Array<VertexElement>& elementList);
private:
};
DEFINE_REF(VertexDeclaration)
class GraphicsPipeline
{
public:
    GraphicsPipeline(PPipelineLayout layout) : layout(layout) {}
    virtual ~GraphicsPipeline(){}
    PPipelineLayout getPipelineLayout() const { return layout; }
protected:
    PPipelineLayout layout;
};
DEFINE_REF(GraphicsPipeline)

class ComputePipeline
{
public:
    ComputePipeline(PPipelineLayout layout) : layout(layout) {}
    virtual ~ComputePipeline(){}
    PPipelineLayout getPipelineLayout() const { return layout; }
protected:
    PPipelineLayout layout;
};
DEFINE_REF(ComputePipeline)

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
    virtual void dispatch(uint32 groupX, uint32 groupY, uint32 groupZ);
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

} // namespace Gfx
} // namespace Seele