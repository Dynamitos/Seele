#include "Buffer.h"
#include "Graphics.h"
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Resources.h"
#include "Metal/MTLResource.hpp"
#include <iostream>

using namespace Seele;
using namespace Seele::Metal;

BufferAllocation::BufferAllocation(PGraphics graphics)
    : CommandBoundResource(graphics) {}

BufferAllocation::~BufferAllocation() {
  if (buffer != nullptr) {
    buffer->release();
  }
}

Buffer::Buffer(PGraphics graphics, uint64 size, void *data, bool dynamic,
               const std::string &name)
    : graphics(graphics), dynamic(dynamic), name(name) {
  createBuffer(size, data);
}

Buffer::~Buffer() {
  for (size_t i = 0; i < buffers.size(); ++i) {
    // TODO
  }
}

void *Buffer::map(bool) { return getHandle()->contents(); }

void *Buffer::mapRegion(uint64 regionOffset, uint64, bool) {
  return (char *)getHandle()->contents() + regionOffset;
}

void Buffer::unmap() {}

void Buffer::rotateBuffer(uint64 size) {
  size = std::max(getSize(), size);
  for (size_t i = 0; i < buffers.size(); ++i) {
    if (buffers[i]->isCurrentlyBound()) {
      continue;
    }
    if (buffers[i]->size < size) {
      buffers[i]->buffer->release();
      buffers[i]->buffer =
          graphics->getDevice()->newBuffer(size, MTL::StorageModeShared);
      buffers[i]->size = size;
    }
    currentBuffer = i;
    return;
  }
  createBuffer(size, nullptr);
  currentBuffer = buffers.size() - 1;
}

void Buffer::createBuffer(uint64 size, void *data) {
  buffers.add(new BufferAllocation(graphics));
  if (data != nullptr) {
    buffers.back()->buffer =
        graphics->getDevice()->newBuffer(data, size, MTL::StorageModeShared);
  } else {
    buffers.back()->buffer =
        graphics->getDevice()->newBuffer(size, MTL::StorageModeShared);
  }
  buffers.back()->buffer->setLabel(
      NS::String::string(name.c_str(), NS::ASCIIStringEncoding));
}

VertexBuffer::VertexBuffer(PGraphics graphics,
                           const VertexBufferCreateInfo &createInfo)
    : Gfx::VertexBuffer(graphics->getFamilyMapping(), createInfo.numVertices,
                        createInfo.vertexSize, createInfo.sourceData.owner),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size,
                           createInfo.sourceData.data, false, createInfo.name) {
}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::updateRegion(DataSource update) {
  void *data = getHandle()->contents();
  std::memcpy((char *)data + update.offset, update.data, update.size);
}

void VertexBuffer::download(Array<uint8> &buffer) {
  void *data = getHandle()->contents();
  buffer.resize(getSize());
  std::memcpy(buffer.data(), data, getSize());
}

void VertexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) {
  currentOwner = newOwner;
}

void VertexBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                                          Gfx::SePipelineStageFlags srcStage,
                                          Gfx::SeAccessFlags dstAccess,
                                          Gfx::SePipelineStageFlags dstStage) {}

IndexBuffer::IndexBuffer(PGraphics graphics,
                         const IndexBufferCreateInfo &createInfo)
    : Gfx::IndexBuffer(graphics->getFamilyMapping(), createInfo.sourceData.size,
                       createInfo.indexType, createInfo.sourceData.owner),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size,
                           createInfo.sourceData.data, false, createInfo.name) {
}

IndexBuffer::~IndexBuffer() {}

void IndexBuffer::download(Array<uint8> &buffer) {
  void *data = getHandle()->contents();
  buffer.resize(getSize());
  std::memcpy(buffer.data(), data, getSize());
}
void IndexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) {
  currentOwner = newOwner;
}

void IndexBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                                         Gfx::SePipelineStageFlags srcStage,
                                         Gfx::SeAccessFlags dstAccess,
                                         Gfx::SePipelineStageFlags dstStage) {}

UniformBuffer::UniformBuffer(PGraphics graphics,
                             const UniformBufferCreateInfo &createInfo)
    : Gfx::UniformBuffer(graphics->getFamilyMapping(), createInfo.sourceData),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size,
                           createInfo.sourceData.data, createInfo.dynamic,
                           createInfo.name) {}

UniformBuffer::~UniformBuffer() {}

bool UniformBuffer::updateContents(const DataSource &sourceData) {
  void *data = getHandle()->contents();
  std::memcpy((char *)data + sourceData.offset, sourceData.data,
              sourceData.size);
  return true;
}

void UniformBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) {
  currentOwner = newOwner;
}

void UniformBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                                           Gfx::SePipelineStageFlags srcStage,
                                           Gfx::SeAccessFlags dstAccess,
                                           Gfx::SePipelineStageFlags dstStage) {
}

ShaderBuffer::ShaderBuffer(PGraphics graphics,
                           const ShaderBufferCreateInfo &createInfo)
    : Gfx::ShaderBuffer(graphics->getFamilyMapping(), createInfo.numElements,
                        createInfo.sourceData),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size,
                           createInfo.sourceData.data, createInfo.dynamic,
                           createInfo.name) {}

ShaderBuffer::~ShaderBuffer() {}

void ShaderBuffer::rotateBuffer(uint64 size) {
  Metal::Buffer::rotateBuffer(size);
}

void ShaderBuffer::updateContents(const ShaderBufferCreateInfo &createInfo) {
  Gfx::ShaderBuffer::updateContents(createInfo);
  if (createInfo.sourceData.data == nullptr) {
    return;
  }
  void *data = map();
  std::memcpy((char *)data + createInfo.sourceData.offset,
              createInfo.sourceData.data, createInfo.sourceData.size);
  unmap();
}

void *ShaderBuffer::mapRegion(uint64 offset, uint64 size, bool writeOnly) {
  return Metal::Buffer::mapRegion(offset, size, writeOnly);
}

void ShaderBuffer::unmap() { Metal::Buffer::unmap(); }

void ShaderBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) {
  currentOwner = newOwner;
}

void ShaderBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess,
                                          Gfx::SePipelineStageFlags srcStage,
                                          Gfx::SeAccessFlags dstAccess,
                                          Gfx::SePipelineStageFlags dstStage) {}
