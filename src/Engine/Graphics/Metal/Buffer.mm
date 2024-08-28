#include "Buffer.h"
#include "Foundation/NSString.hpp"
#include "Graphics.h"
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Resources.h"
#include "Metal/MTLResource.hpp"
#include <iostream>

using namespace Seele;
using namespace Seele::Metal;

BufferAllocation::BufferAllocation(PGraphics graphics, const std::string& name, uint64 size, MTL::ResourceOptions options)
    : CommandBoundResource(graphics) {
    buffer = graphics->getDevice()->newBuffer(size, options);
    buffer->setLabel(NS::String::string(name.c_str(), NS::ASCIIStringEncoding));
}

BufferAllocation::~BufferAllocation() {
    if (buffer != nullptr) {
        buffer->release();
    }
}

void BufferAllocation::pipelineBarrier(Gfx::SeAccessFlags, Gfx::SePipelineStageFlags, Gfx::SeAccessFlags, Gfx::SePipelineStageFlags) {}

void BufferAllocation::transferOwnership(Gfx::QueueType) {}

void BufferAllocation::updateContents(uint64 regionOffset, uint64 regionSize, void* ptr) {
    std::memcpy((uint8*)buffer->contents() + regionOffset, ptr, regionSize);
}

void BufferAllocation::readContents(uint64 regionOffset, uint64 regionSize, void* ptr) {
    std::memcpy(ptr, (uint8*)buffer->contents() + regionOffset, regionSize);
}

void* BufferAllocation::map() { return buffer->contents(); }

void BufferAllocation::unmap() {}

Buffer::Buffer(PGraphics graphics, uint64 size, Gfx::SeBufferUsageFlags usage, Gfx::QueueType queueType, bool dynamic, std::string name,
               bool createCleared, uint32 clearValue)
    : graphics(graphics), currentBuffer(0), dynamic(dynamic), createCleared(createCleared), name(name), clearValue(clearValue) {
    buffers.add(nullptr);
    createBuffer(size, 0);
}

Buffer::~Buffer() {
    for (size_t i = 0; i < buffers.size(); ++i) {
        // TODO
    }
}

void Buffer::updateContents(uint64 regionOffset, uint64 regionSize, void* ptr) {
    getAlloc()->updateContents(regionOffset, regionSize, ptr);
}

void Buffer::readContents(uint64 regionOffset, uint64 regionSize, void* ptr) { getAlloc()->readContents(regionOffset, regionSize, ptr); }

void* Buffer::map() { return getAlloc()->map(); }

void Buffer::unmap() { return getAlloc()->unmap(); }

void Buffer::rotateBuffer(uint64 size, bool preserveContents) {
    if (size == 0)
        return;
    assert(dynamic);
    for (uint32 i = 0; i < buffers.size(); ++i) {
        if (buffers[i]->isCurrentlyBound()) {
            continue;
        }
        if (buffers[i]->size < size) {
            createBuffer(size, i);
        }
        if (preserveContents) {
            copyBuffer(currentBuffer, i);
        }
        currentBuffer = i;
        return;
    }
    buffers.add(nullptr);
    createBuffer(size, buffers.size() - 1);
    if (preserveContents) {
        copyBuffer(currentBuffer, buffers.size() - 1);
    }
    currentBuffer = buffers.size() - 1;
}

void Buffer::createBuffer(uint64 size, uint32 destIndex) {
    buffers[destIndex] = new BufferAllocation(graphics, name, size);
    if (createCleared) {
        std::memset(buffers[destIndex]->map(), clearValue, buffers[destIndex]->size);
    }
}

void Buffer::copyBuffer(uint64 src, uint64 dest) {
    if (src == dest) {
        return;
    }
    std::memcpy(buffers[dest]->map(), buffers[src]->map(), buffers[src]->size);
}

void Buffer::transferOwnership(Gfx::QueueType newOwner) { getAlloc()->transferOwnership(newOwner); }

void Buffer::pipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                             Gfx::SePipelineStageFlags dstStage) {
    getAlloc()->pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

VertexBuffer::VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo& createInfo)
    : Gfx::VertexBuffer(graphics->getFamilyMapping(), createInfo),
      Metal::Buffer(graphics, createInfo.sourceData.size, Gfx::SE_BUFFER_USAGE_VERTEX_BUFFER_BIT, createInfo.sourceData.owner, false,
                    createInfo.name) {
    if (createInfo.sourceData.size > 0 && createInfo.sourceData.data != nullptr) {
        getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    }
}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::updateRegion(uint64 offset, uint64 size, void* data) { getAlloc()->updateContents(offset, size, data); }

void VertexBuffer::download(Array<uint8>& buffer) {
    void* data = getHandle()->contents();
    buffer.resize(getSize());
    std::memcpy(buffer.data(), data, getSize());
}

void VertexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Metal::Buffer::transferOwnership(newOwner); }

void VertexBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                          Gfx::SePipelineStageFlags dstStage) {
    Metal::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

IndexBuffer::IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo& createInfo)
    : Gfx::IndexBuffer(graphics->getFamilyMapping(), createInfo),
      Metal::Buffer(graphics, createInfo.sourceData.size,
                    Gfx::SE_BUFFER_USAGE_INDEX_BUFFER_BIT | Gfx::SE_BUFFER_USAGE_TRANSFER_DST_BIT | Gfx::SE_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    createInfo.sourceData.owner, false, createInfo.name) {}

IndexBuffer::~IndexBuffer() {}

void IndexBuffer::download(Array<uint8>& buffer) {
    void* data = getHandle()->contents();
    buffer.resize(getSize());
    std::memcpy(buffer.data(), data, getSize());
}
void IndexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Metal::Buffer::transferOwnership(newOwner); }

void IndexBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                         Gfx::SePipelineStageFlags dstStage) {
    Metal::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

UniformBuffer::UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo& createInfo)
    : Gfx::UniformBuffer(graphics->getFamilyMapping(), createInfo),
      Metal::Buffer(graphics, createInfo.sourceData.size, Gfx::SE_BUFFER_USAGE_UNIFORM_BUFFER_BIT, createInfo.sourceData.owner,
                    createInfo.dynamic, createInfo.name) {
    if (createInfo.sourceData.size > 0 && createInfo.sourceData.data != nullptr) {
        getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    }
}

UniformBuffer::~UniformBuffer() {}

void UniformBuffer::rotateBuffer(uint64 size) { Metal::Buffer::rotateBuffer(size); }

void UniformBuffer::updateContents(uint64 offset, uint64 size, void* data) {
    if (size > 0 && data != nullptr) {
        Metal::Buffer::rotateBuffer(size);
        getAlloc()->updateContents(offset, size, data);
    }
}

void UniformBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Metal::Buffer::transferOwnership(newOwner); }

void UniformBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                           Gfx::SePipelineStageFlags dstStage) {
    Metal::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

ShaderBuffer::ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo& createInfo)
    : Gfx::ShaderBuffer(graphics->getFamilyMapping(), createInfo),
      Seele::Metal::Buffer(graphics, createInfo.sourceData.size, Gfx::SE_BUFFER_USAGE_STORAGE_BUFFER_BIT, createInfo.sourceData.owner,
                           createInfo.dynamic, createInfo.name, createInfo.createCleared, createInfo.clearValue) {}

ShaderBuffer::~ShaderBuffer() {}

void ShaderBuffer::readContents(uint64 offset, uint64 size, void* data) { getAlloc()->readContents(offset, size, data); }

void ShaderBuffer::updateContents(uint64 offset, uint64 size, void* data) {
    if (data == nullptr) {
        return;
    }
    // We always want to update, as the contents could be different on the GPU
    if (size > 0 && data != nullptr) {
        getAlloc()->updateContents(offset, size, data);
    }
}

void ShaderBuffer::rotateBuffer(uint64 size, bool preserveContents) {
    assert(dynamic);
    Metal::Buffer::rotateBuffer(size, preserveContents);
}

void* ShaderBuffer::map() { return Metal::Buffer::map(); }

void ShaderBuffer::unmap() { Metal::Buffer::unmap(); }

void ShaderBuffer::clear() { std::memset(getAlloc()->map(), 0, getSize()); }

void ShaderBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Metal::Buffer::transferOwnership(newOwner); }

void ShaderBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                          Gfx::SePipelineStageFlags dstStage) {
    Metal::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
