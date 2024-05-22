#pragma once
#include "Graphics.h"
#include "Graphics/Buffer.h"
#include "Resources.h"

namespace Seele {
namespace Vulkan {
DECLARE_REF(Command)
DECLARE_REF(Fence)
class BufferAllocation : public CommandBoundResource {
public:
  BufferAllocation(PGraphics graphics);
  virtual ~BufferAllocation();
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VmaAllocation();
  VmaAllocationInfo info = VmaAllocationInfo();
  VkMemoryPropertyFlags properties = 0;
  uint64 size = 0;
};
DEFINE_REF(BufferAllocation);
class Buffer {
public:
  Buffer(PGraphics graphics, uint64 size, VkBufferUsageFlags usage,
         Gfx::QueueType &queueType, bool dynamic, std::string name);
  virtual ~Buffer();
  VkBuffer getHandle() const { return buffers[currentBuffer]->buffer; }
  PBufferAllocation getAlloc() const { return buffers[currentBuffer]; }
  uint64 getSize() const { return buffers[currentBuffer]->size; }
  void *map(bool writeOnly = true);
  void *mapRegion(uint64 regionOffset, uint64 regionSize,
                  bool writeOnly = true);
  void unmap();

protected:
  PGraphics graphics;
  uint32 currentBuffer;
  Gfx::QueueType &owner;
  Array<OBufferAllocation> buffers;
  VkBufferUsageFlags usage;
  bool dynamic;
  std::string name;
  void rotateBuffer(uint64 size);
  void createBuffer(uint64 size);

  void executeOwnershipBarrier(Gfx::QueueType newOwner);
  void executePipelineBarrier(VkAccessFlags srcAccess,
                              VkPipelineStageFlags srcStage,
                              VkAccessFlags dstAccess,
                              VkPipelineStageFlags dstStage);

  virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) = 0;

  virtual VkAccessFlags getSourceAccessMask() = 0;
  virtual VkAccessFlags getDestAccessMask() = 0;
};
DEFINE_REF(Buffer)

class VertexBuffer : public Gfx::VertexBuffer, public Buffer {
public:
  VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo &sourceData);
  virtual ~VertexBuffer();

  virtual void updateRegion(DataSource update) override;
  virtual void download(Array<uint8> &buffer) override;

protected:
  // Inherited via Vulkan::Buffer
  virtual VkAccessFlags getSourceAccessMask() override;
  virtual VkAccessFlags getDestAccessMask() override;
  virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void
  executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                         Gfx::SePipelineStageFlags srcStage,
                         Gfx::SeAccessFlags dstAccess,
                         Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(VertexBuffer)

class IndexBuffer : public Gfx::IndexBuffer, public Buffer {
public:
  IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo &sourceData);
  virtual ~IndexBuffer();

  virtual void download(Array<uint8> &buffer) override;

protected:
  // Inherited via Vulkan::Buffer
  virtual VkAccessFlags getSourceAccessMask() override;
  virtual VkAccessFlags getDestAccessMask() override;
  virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void
  executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                         Gfx::SePipelineStageFlags srcStage,
                         Gfx::SeAccessFlags dstAccess,
                         Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(IndexBuffer)
class UniformBuffer : public Gfx::UniformBuffer, public Buffer {
public:
  UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo &sourceData);
  virtual ~UniformBuffer();
  virtual void updateContents(const DataSource &sourceData) override;
  virtual void rotateBuffer(uint64 size) override;

protected:
  // Inherited via Vulkan::Buffer
  virtual VkAccessFlags getSourceAccessMask() override;
  virtual VkAccessFlags getDestAccessMask() override;
  virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void
  executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                         Gfx::SePipelineStageFlags srcStage,
                         Gfx::SeAccessFlags dstAccess,
                         Gfx::SePipelineStageFlags dstStage) override;

private:
};
DEFINE_REF(UniformBuffer)

class ShaderBuffer : public Gfx::ShaderBuffer, public Buffer {
public:
  ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo &sourceData);
  virtual ~ShaderBuffer();
  virtual void
  updateContents(const ShaderBufferCreateInfo &createInfo) override;
  virtual void rotateBuffer(uint64 size) override;
  virtual void *mapRegion(uint64 offset, uint64 size, bool writeOnly) override;
  virtual void unmap() override;

protected:
  // Inherited via Vulkan::Buffer
  virtual VkAccessFlags getSourceAccessMask() override;
  virtual VkAccessFlags getDestAccessMask() override;
  virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void
  executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                         Gfx::SePipelineStageFlags srcStage,
                         Gfx::SeAccessFlags dstAccess,
                         Gfx::SePipelineStageFlags dstStage) override;

private:
};
DEFINE_REF(ShaderBuffer)

} // namespace Vulkan
} // namespace Seele