#include "Buffer.h"
#include "Initializer.h"

using namespace Seele;
using namespace Seele::Gfx;

Buffer::Buffer(QueueFamilyMapping mapping) : QueueOwnedResource(mapping) {}

Buffer::~Buffer() {}

VertexBuffer::VertexBuffer(QueueFamilyMapping mapping, const VertexBufferCreateInfo& createInfo)
    : Buffer(mapping), numVertices(createInfo.numVertices), vertexSize(createInfo.vertexSize) {}
VertexBuffer::~VertexBuffer() {}

IndexBuffer::IndexBuffer(QueueFamilyMapping mapping, const IndexBufferCreateInfo& createInfo)
    : Buffer(mapping), indexType(createInfo.indexType) {
    switch (indexType) {
    case SE_INDEX_TYPE_UINT16:
        numIndices = createInfo.sourceData.size / sizeof(uint16);
        break;
    case SE_INDEX_TYPE_UINT32:
        numIndices = createInfo.sourceData.size  / sizeof(uint32);
        break;
    default:
        break;
    }
}
IndexBuffer::~IndexBuffer() {}

UniformBuffer::UniformBuffer(QueueFamilyMapping mapping, const UniformBufferCreateInfo&) : Buffer(mapping) {}

UniformBuffer::~UniformBuffer() {}

ShaderBuffer::ShaderBuffer(QueueFamilyMapping mapping, const ShaderBufferCreateInfo& createInfo)
    : Buffer(mapping), numElements(createInfo.numElements) {}

ShaderBuffer::~ShaderBuffer() {}
