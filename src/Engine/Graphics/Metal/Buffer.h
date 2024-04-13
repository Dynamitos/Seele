#pragma once
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Resources.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class Buffer {
public:
  Buffer(PGraphics graphics, uint64 size, void *data, bool dynamic);
  virtual ~Buffer();
  MTL::Buffer *getHandle() const { return buffers[currentBuffer]; }
  uint64 getSize() const { return size; }
  void advanceBuffer() { currentBuffer = (currentBuffer + 1) % numBuffers; }
  void *map(bool writeOnly = true);
  void *mapRegion(uint64 regionOffset, uint64 regionSize, bool writeOnly);
  void unmap();

protected:
  PGraphics graphics;
  uint32 currentBuffer;
  uint64 size;
  MTL::Buffer *buffers[Gfx::numFramesBuffered];
  uint32 numBuffers;
};
DEFINE_REF(Buffer)
class VertexBuffer : public Gfx::VertexBuffer, public Buffer {
public:
  VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo &createInfo);
  virtual ~VertexBuffer();
  virtual void updateRegion(DataSource update) override;
  virtual void download(Array<uint8> &buffer) override;

protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                      Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(VertexBuffer)
class IndexBuffer : public Gfx::IndexBuffer, public Buffer {
public:
  IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo &createInfo);
  virtual ~IndexBuffer();

  virtual void download(Array<uint8> &buffer) override;
protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                      Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(IndexBuffer)
DEFINE_REF(IndexBuffer)
class UniformBuffer : public Gfx::UniformBuffer, public Buffer {
public:
  UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo &createInfo);
  virtual ~UniformBuffer();

  virtual bool updateContents(const DataSource &sourceData) override;

protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                      Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(UniformBuffer)
class ShaderBuffer : public Gfx::ShaderBuffer, public Buffer {
public:
  ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo &createInfo);
  virtual ~ShaderBuffer();

  virtual bool updateContents(const DataSource &sourceData) override;

protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
  virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                      Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(ShaderBuffer)
} // namespace Metal
} // namespace Seele