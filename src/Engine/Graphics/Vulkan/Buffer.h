#pragma once
#include "Graphics.h"
#include "Graphics/Buffer.h"

namespace Seele {
namespace Vulkan {
DECLARE_REF(Command)
DECLARE_REF(Fence)
class Buffer {
public:
  Buffer(PGraphics graphics, uint64 size, VkBufferUsageFlags usage,
         Gfx::QueueType &queueType, bool dynamic, std::string name);
  virtual ~Buffer();
  VkBuffer getHandle() const { return buffers[currentBuffer].buffer; }
  uint64 getSize() const { return size; }
  void *map(bool writeOnly = true);
  void *mapRegion(uint64 regionOffset, uint64 regionSize,
                  bool writeOnly = true);
  void unmap();

protected:
  struct BufferAllocation {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    VkMemoryPropertyFlags properties;
  };
  PGraphics graphics;
  uint32 currentBuffer;
  uint64 size;
  Gfx::QueueType &owner;
  Array<BufferAllocation> buffers;
  VkBufferUsageFlags usage;
  uint32 numBuffers;
  std::string name;
  void createBuffer();

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
  virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
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
  virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                                      Gfx::SePipelineStageFlags srcStage,
                                      Gfx::SeAccessFlags dstAccess,
                                      Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(IndexBuffer)
class UniformBuffer : public Gfx::UniformBuffer, public Buffer {
public:
  UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo &sourceData);
  virtual ~UniformBuffer();
  virtual bool updateContents(const DataSource &sourceData) override;

protected:
  // Inherited via Vulkan::Buffer
  virtual VkAccessFlags getSourceAccessMask() override;
  virtual VkAccessFlags getDestAccessMask() override;
  virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
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
  virtual bool updateContents(const DataSource &sourceData) override;

protected:
  // Inherited via Vulkan::Buffer
  virtual VkAccessFlags getSourceAccessMask() override;
  virtual VkAccessFlags getDestAccessMask() override;
  virtual void requestOwnershipTransfer(Gfx::QueueType newOwner) override;
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                                      Gfx::SePipelineStageFlags srcStage,
                                      Gfx::SeAccessFlags dstAccess,
                                      Gfx::SePipelineStageFlags dstStage) override;

private:
};
DEFINE_REF(ShaderBuffer)

} // namespace Vulkan
} // namespace Seele