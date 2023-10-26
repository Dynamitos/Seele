#pragma once
#include "Graphics/Buffer.h"
#include "Graphics.h"
#include "Allocator.h"

namespace Seele
{
namespace Vulkan
{

class ShaderBuffer
{
public:
    ShaderBuffer(PGraphics graphics, uint64 size, VkBufferUsageFlags usage, Gfx::QueueType& queueType, bool bDynamic = false);
    virtual ~ShaderBuffer();
    VkBuffer getHandle() const
    {
        return buffers[currentBuffer].buffer;
    }
    uint64 getSize() const
    {
        return size;
    }
    VkDeviceSize getOffset() const;
    void advanceBuffer()
    {
        currentBuffer = (currentBuffer + 1) % numBuffers;
    }
    virtual void *lock(bool bWriteOnly = true);
    virtual void *lockRegion(uint64 regionOffset, uint64 regionSize, bool bWriteOnly = true);
    virtual void unlock();

protected:
    struct BufferAllocation
    {
        VkBuffer buffer;
        PSubAllocation allocation;
    };
    PGraphics graphics;
    uint32 currentBuffer;
    uint64 size;
    Gfx::QueueType& owner;
    BufferAllocation buffers[Gfx::numFramesBuffered];
    uint32 numBuffers;

    void executeOwnershipBarrier(Gfx::QueueType newOwner);
    void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
        
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) = 0;

    virtual VkAccessFlags getSourceAccessMask() = 0;
    virtual VkAccessFlags getDestAccessMask() = 0;
};
DEFINE_REF(ShaderBuffer)

DECLARE_REF(StagingBuffer)
class UniformBuffer : public Gfx::UniformBuffer, public ShaderBuffer
{
public:
    UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo &resourceData);
    virtual ~UniformBuffer();
    virtual bool updateContents(const BulkResourceData &resourceData);
    
    virtual void* lock(bool bWriteOnly = true) override;
    virtual void unlock() override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask();
    virtual VkAccessFlags getDestAccessMask();
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner);
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);

private:
    PStagingBuffer dedicatedStagingBuffer;
};
DEFINE_REF(UniformBuffer)

class ShaderBuffer : public Gfx::ShaderBuffer, public ShaderBuffer
{
public:
    ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo &resourceData);
    virtual ~ShaderBuffer();
    virtual bool updateContents(const BulkResourceData &resourceData);

    virtual void* lock(bool bWriteOnly = true) override;
    virtual void unlock() override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask();
    virtual VkAccessFlags getDestAccessMask();
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner);
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
private:
    PStagingBuffer dedicatedStagingBuffer;
};
DEFINE_REF(ShaderBuffer)

class VertexBuffer : public Gfx::VertexBuffer, public ShaderBuffer
{
public:
    VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo &resourceData);
    virtual ~VertexBuffer();

    virtual void updateRegion(BulkResourceData update) override;
    virtual void download(Array<uint8>& buffer) override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask();
    virtual VkAccessFlags getDestAccessMask();
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner);
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
};
DEFINE_REF(VertexBuffer)

class IndexBuffer : public Gfx::IndexBuffer, public ShaderBuffer
{
public:
    IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo &resourceData);
    virtual ~IndexBuffer();

    virtual void download(Array<uint8>& buffer) override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask();
    virtual VkAccessFlags getDestAccessMask();
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner);
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
};
DEFINE_REF(IndexBuffer)

}
}