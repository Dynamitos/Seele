#include "Buffer.h"
#include "Graphics.h"
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Metal/MTLResource.hpp"

using namespace Seele;
using namespace Seele::Metal;

Buffer::Buffer(PGraphics graphics, uint64 size, void *data, bool dynamic)
    : graphics(graphics), size(size) {
  if (dynamic) {
    numBuffers = Gfx::numFramesBuffered;
  } else {
    numBuffers = 1;
  }
  for (size_t i = 0; i < numBuffers; ++i) {
    if (data != nullptr) {
      buffers[i] = graphics->getDevice()->newBuffer(
          data, size, MTL::ResourceOptionCPUCacheModeDefault);
    } else {
      buffers[i] = graphics->getDevice()->newBuffer(
          size, MTL::ResourceOptionCPUCacheModeDefault);
    }
  }
}

Buffer::~Buffer() {
  for (size_t i = 0; i < numBuffers; ++i) {
    buffers[i]->release();
  }
}

void *Buffer::map(bool) { return getHandle()->contents(); }

void *Buffer::mapRegion(uint64 regionOffset, uint64, bool) {
  return (char *)getHandle()->contents() + regionOffset;
}

void unmap() {}

VertexBuffer::VertexBuffer(PGraphics graphics,
                           const VertexBufferCreateInfo &createInfo)
    : Gfx::VertexBuffer(graphics->getFamilyMapping(), createInfo.numVertices,
                        createInfo.vertexSize, createInfo.sourceData.owner),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size,
                           createInfo.sourceData.data, false) {}

VertexBuffer::~VertexBuffer() {}

IndexBuffer::IndexBuffer(PGraphics graphics,
                         const IndexBufferCreateInfo &createInfo)
    : Gfx::IndexBuffer(graphics->getFamilyMapping(), createInfo.sourceData.size,
                       createInfo.indexType, createInfo.sourceData.owner),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size,
                           createInfo.sourceData.data, false) {}

IndexBuffer::~IndexBuffer() {}

UniformBuffer::UniformBuffer(PGraphics graphics,
                             const UniformBufferCreateInfo &createInfo)
    : Gfx::UniformBuffer(graphics->getFamilyMapping(), createInfo.sourceData),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size,
                           createInfo.sourceData.data, createInfo.dynamic) {}

UniformBuffer::~UniformBuffer() {}

ShaderBuffer::ShaderBuffer(PGraphics graphics,
                           const ShaderBufferCreateInfo &createInfo)
    : Gfx::ShaderBuffer(graphics->getFamilyMapping(), createInfo.numElements,
                        createInfo.sourceData),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size,
                           createInfo.sourceData.data, createInfo.dynamic) {}

ShaderBuffer::~ShaderBuffer() {}
