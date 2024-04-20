#include "Buffer.h"
#include "Graphics.h"
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include <iostream>

using namespace Seele;
using namespace Seele::Metal;

Buffer::Buffer(PGraphics graphics, uint64 size, void* data, bool dynamic, const std::string& name) : graphics(graphics), size(size) {
  if (dynamic) {
    numBuffers = Gfx::numFramesBuffered;
  } else {
    numBuffers = 1;
  }
  for (size_t i = 0; i < numBuffers; ++i) {
    if (data != nullptr) {
      buffers[i] = graphics->getDevice()->newBuffer(data, size, MTL::StorageModeShared);
    } else {
      buffers[i] = graphics->getDevice()->newBuffer(size, MTL::StorageModeShared);
    }
      buffers[i]->setLabel(NS::String::string(name.c_str(), NS::ASCIIStringEncoding));
  }
}

Buffer::~Buffer() {
  for (size_t i = 0; i < numBuffers; ++i) {
    buffers[i]->release();
  }
}

void* Buffer::map(bool) { return getHandle()->contents(); }

void* Buffer::mapRegion(uint64 regionOffset, uint64, bool) { return (char*)getHandle()->contents() + regionOffset; }

void Buffer::unmap() {}

VertexBuffer::VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo& createInfo)
    : Gfx::VertexBuffer(graphics->getFamilyMapping(), createInfo.numVertices, createInfo.vertexSize,
                        createInfo.sourceData.owner),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size, createInfo.sourceData.data, false, createInfo.name) {}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::updateRegion(DataSource update) {
  void* data = getHandle()->contents();
  std::memcpy((char*)data + update.offset, update.data, update.size);
}

void VertexBuffer::download(Array<uint8>& buffer) {
  void* data = getHandle()->contents();
  buffer.resize(size);
  std::memcpy(buffer.data(), data, size);
}

void VertexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { currentOwner = newOwner; }

void VertexBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                          Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
  
}

IndexBuffer::IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo& createInfo)
    : Gfx::IndexBuffer(graphics->getFamilyMapping(), createInfo.sourceData.size, createInfo.indexType,
                       createInfo.sourceData.owner),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size, createInfo.sourceData.data, false, createInfo.name) {}

IndexBuffer::~IndexBuffer() {}

void IndexBuffer::download(Array<uint8>& buffer) {
  void* data = getHandle()->contents();
  buffer.resize(size);
  std::memcpy(buffer.data(), data, size);
}
void IndexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { currentOwner = newOwner; }

void IndexBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                          Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {}

UniformBuffer::UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo& createInfo)
    : Gfx::UniformBuffer(graphics->getFamilyMapping(), createInfo.sourceData),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size, createInfo.sourceData.data, createInfo.dynamic, createInfo.name) {}

UniformBuffer::~UniformBuffer() {}

bool UniformBuffer::updateContents(const DataSource& sourceData) {
  void* data = getHandle()->contents();
  std::memcpy((char*)data + sourceData.offset, sourceData.data, sourceData.size);
  return true;
}

void UniformBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { currentOwner = newOwner; }

void UniformBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                          Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {}

ShaderBuffer::ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo& createInfo)
    : Gfx::ShaderBuffer(graphics->getFamilyMapping(), createInfo.numElements, createInfo.sourceData),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size, createInfo.sourceData.data, createInfo.dynamic, createInfo.name) {}

ShaderBuffer::~ShaderBuffer() {}

bool ShaderBuffer::updateContents(const DataSource& sourceData) {
  void* data = getHandle()->contents();
  std::memcpy((char*)data + sourceData.offset, sourceData.data, sourceData.size);
  return true;
}

void ShaderBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { currentOwner = newOwner; }


void ShaderBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                          Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {}
