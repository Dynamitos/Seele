#pragma once
#include "Resources.h"
#include "Initializer.h"

namespace Seele {
namespace Gfx {
class Buffer : public QueueOwnedResource {
public:
  Buffer(QueueFamilyMapping mapping, QueueType startQueueType);
  virtual ~Buffer();

protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
};

class VertexBuffer : public Buffer {
public:
  VertexBuffer(QueueFamilyMapping mapping, uint32 numVertices,
               uint32 vertexSize, QueueType startQueueType);
  virtual ~VertexBuffer();
  constexpr uint32 getNumVertices() const { return numVertices; }
  // Size of one vertex in bytes
  constexpr uint32 getVertexSize() const { return vertexSize; }

  virtual void updateRegion(DataSource update) = 0;
  virtual void download(Array<uint8> &buffer) = 0;

protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
  virtual void executePipelineBarrier(SeAccessFlags srcAccess,
                                      SePipelineStageFlags srcStage,
                                      SeAccessFlags dstAccess,
                                      SePipelineStageFlags dstStage) = 0;
  uint32 numVertices;
  uint32 vertexSize;
};
DEFINE_REF(VertexBuffer)

class IndexBuffer : public Buffer {
public:
  IndexBuffer(QueueFamilyMapping mapping, uint64 size, Gfx::SeIndexType index,
              QueueType startQueueType);
  virtual ~IndexBuffer();
  constexpr uint64 getNumIndices() const { return numIndices; }
  constexpr Gfx::SeIndexType getIndexType() const { return indexType; }

  virtual void download(Array<uint8> &buffer) = 0;

protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
  virtual void executePipelineBarrier(SeAccessFlags srcAccess,
                                      SePipelineStageFlags srcStage,
                                      SeAccessFlags dstAccess,
                                      SePipelineStageFlags dstStage) = 0;
  Gfx::SeIndexType indexType;
  uint64 numIndices;
};
DEFINE_REF(IndexBuffer)

DECLARE_REF(UniformBuffer)
class UniformBuffer : public Buffer {
public:
  UniformBuffer(QueueFamilyMapping mapping, const DataSource &sourceData);
  virtual ~UniformBuffer();

  virtual void rotateBuffer(uint64 size) = 0;
  virtual void updateContents(const DataSource &sourceData) = 0;

protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
  virtual void executePipelineBarrier(SeAccessFlags srcAccess,
                                      SePipelineStageFlags srcStage,
                                      SeAccessFlags dstAccess,
                                      SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(UniformBuffer)
class ShaderBuffer : public Buffer {
public:
  ShaderBuffer(QueueFamilyMapping mapping, uint32 numElements,
               const DataSource &bulkResourceData);
  virtual ~ShaderBuffer();
  virtual void rotateBuffer(uint64 size, bool preserveContents = false) = 0;
  virtual void updateContents(const ShaderBufferCreateInfo &sourceData) = 0;
  constexpr uint32 getNumElements() const { return numElements; }
  virtual void *mapRegion(uint64 offset = 0, uint64 size = -1,
                          bool writeOnly = true) = 0;
  virtual void unmap() = 0;

  virtual void clear() = 0;

protected:
  // Inherited via QueueOwnedResource
  virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
  virtual void executePipelineBarrier(SeAccessFlags srcAccess,
                                      SePipelineStageFlags srcStage,
                                      SeAccessFlags dstAccess,
                                      SePipelineStageFlags dstStage) = 0;

  uint32 numElements;
};
DEFINE_REF(ShaderBuffer)
} // namespace Gfx
} // namespace Seele