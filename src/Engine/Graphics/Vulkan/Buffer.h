#pragma once
#include "Graphics/Buffer.h"
#include "Graphics.h"

namespace Seele
{
namespace Vulkan
{
class Buffer
{
public:
    Buffer(PGraphics graphics, 
        uint64 size, 
        VkBufferUsageFlags usage, 
        Gfx::QueueType& queueType, 
        bool dynamic,
        std::string name);
    virtual ~Buffer();
    VkBuffer getHandle() const
    {
        return buffers[currentBuffer].buffer;
    }
    uint64 getSize() const
    {
        return size;
    }
    void advanceBuffer()
    {
        currentBuffer = (currentBuffer + 1) % numBuffers;
    }
    void *map(bool writeOnly = true);
    void *mapRegion(uint64 regionOffset, uint64 regionSize, bool writeOnly = true);
    void unmap();

protected:
    struct BufferAllocation
    {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo info;
        VkMemoryPropertyFlags properties;
    };
    PGraphics graphics;
    uint32 currentBuffer;
    uint64 size;
    Gfx::QueueType& owner;
    BufferAllocation buffers[Gfx::numFramesBuffered];
    uint32 numBuffers;
    std::string name;

    void executeOwnershipBarrier(Gfx::QueueType newOwner);
    void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
        
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) = 0;

    virtual VkAccessFlags getSourceAccessMask() = 0;
    virtual VkAccessFlags getDestAccessMask() = 0;
};
DEFINE_REF(Buffer)

class UniformBuffer : public Gfx::UniformBuffer, public Buffer
{
public:
    UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo &sourceData);
    virtual ~UniformBuffer();
    virtual bool updateContents(const DataSource &sourceData) override;
    
    virtual void beginFrame() override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask() override;
    virtual VkAccessFlags getDestAccessMask() override;
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) override;

private:
};
DEFINE_REF(UniformBuffer)

class ShaderBuffer : public Gfx::ShaderBuffer, public Buffer
{
public:
    ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo &sourceData);
    virtual ~ShaderBuffer();
    virtual bool updateContents(const DataSource &sourceData) override;
    virtual void beginFrame() override;

protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask() override;
    virtual VkAccessFlags getDestAccessMask() override;
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) override;
private:
};
DEFINE_REF(ShaderBuffer)

class VertexBuffer : public Gfx::VertexBuffer, public Buffer
{
public:
    VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo &sourceData);
    virtual ~VertexBuffer();

    virtual void updateRegion(DataSource update) override;
    virtual void download(Array<uint8>& buffer) override;
    virtual void beginFrame() override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask() override;
    virtual VkAccessFlags getDestAccessMask() override;
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) override;
};
DEFINE_REF(VertexBuffer)

class IndexBuffer : public Gfx::IndexBuffer, public Buffer
{
public:
    IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo &sourceData);
    virtual ~IndexBuffer();

    virtual void download(Array<uint8>& buffer) override;
    virtual void beginFrame() override;
protected:
    // Inherited via Vulkan::Buffer
    virtual VkAccessFlags getSourceAccessMask() override;
    virtual VkAccessFlags getDestAccessMask() override;
    virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) override;
};
DEFINE_REF(IndexBuffer)
}
}