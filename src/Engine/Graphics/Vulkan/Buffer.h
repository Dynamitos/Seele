#pragma once
#include "Graphics.h"
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Resources.h"

namespace Seele {
namespace Vulkan {
DECLARE_REF(Command)
DECLARE_REF(Fence)

class BufferAllocation : public CommandBoundResource {
  public:
    BufferAllocation(PGraphics graphics, const std::string& name, VkBufferCreateInfo bufferInfo, VmaAllocationCreateInfo allocInfo,
                     Gfx::QueueType owner, uint64 alignment = 0);
    virtual ~BufferAllocation();
    void pipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
    void transferOwnership(Gfx::QueueType newOwner);
    void updateContents(uint64 regionOffset, uint64 regionSize, void* ptr);
    void readContents(uint64 regionOffset, uint64 regionSize, void* ptr);
    void* map();
    void unmap();
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VmaAllocation();
    VmaAllocationInfo info = VmaAllocationInfo();
    VkMemoryPropertyFlags properties = 0;
    void* mappedPointer = nullptr;
    uint64 size = 0;
    VkDeviceAddress deviceAddress;
    Gfx::QueueType owner;
};
DEFINE_REF(BufferAllocation);
class Buffer {
  public:
    Buffer(PGraphics graphics, uint64 size, VkBufferUsageFlags usage, Gfx::QueueType initialOwner, bool dynamic, std::string name,
           bool createCleared = false, uint32 clearValue = 0);
    virtual ~Buffer();
    VkBuffer getHandle() const { return buffers[currentBuffer]->buffer; }
    VkDeviceAddress getDeviceAddress() const { return buffers[currentBuffer]->deviceAddress; }
    PBufferAllocation getAlloc() const {
        if (buffers.empty())
            return nullptr;
         return buffers[currentBuffer];
    }
    uint64 getSize() const { return buffers[currentBuffer]->size; }
    void updateContents(uint64 regionOffset, uint64 regionSize, void* ptr);
    void readContents(uint64 regionOffset, uint64 regionSize, void* ptr);
    void* map();
    void unmap();

  protected:
    PGraphics graphics;
    uint32 currentBuffer;
    Gfx::QueueType initialOwner;
    Array<OBufferAllocation> buffers;
    OBufferAllocation stagingBuffer = nullptr;
    VkBufferUsageFlags usage;
    bool dynamic;
    bool createCleared;
    std::string name;
    uint32 clearValue;
    void rotateBuffer(uint64 size, bool preserveContents = false);
    void createBuffer(uint64 size, uint32 destIndex);
    void copyBuffer(uint64 src, uint64 dest);

    void transferOwnership(Gfx::QueueType newOwner);
    void pipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
};
DEFINE_REF(Buffer)

class VertexBuffer : public Gfx::VertexBuffer, public Buffer {
  public:
    VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo& sourceData);
    virtual ~VertexBuffer();

    virtual void updateRegion(uint64 offset, uint64 size, void* data) override;
    virtual void download(Array<uint8>& buffer) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(VertexBuffer)

class IndexBuffer : public Gfx::IndexBuffer, public Buffer {
  public:
    IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo& sourceData);
    virtual ~IndexBuffer();

    virtual void download(Array<uint8>& buffer) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(IndexBuffer)
class UniformBuffer : public Gfx::UniformBuffer, public Buffer {
  public:
    UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo& sourceData);
    virtual ~UniformBuffer();
    virtual void updateContents(uint64 offset, uint64 size, void* data) override;
    virtual void rotateBuffer(uint64 size) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;

  private:
};
DEFINE_REF(UniformBuffer)

class ShaderBuffer : public Gfx::ShaderBuffer, public Buffer {
  public:
    ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo& sourceData);
    virtual ~ShaderBuffer();
    virtual void readContents(uint64 offset, uint64 size, void* data) override;
    virtual void updateContents(uint64 offset, uint64 size, void* data) override;
    virtual void rotateBuffer(uint64 size, bool preserveContents = false) override;
    virtual void* map() override;
    virtual void unmap() override;

    virtual void clear() override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;

  private:
};
DEFINE_REF(ShaderBuffer)

} // namespace Vulkan
} // namespace Seele