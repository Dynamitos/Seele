#include "Buffer.h"
#include "Command.h"
#include "Enums.h"
#include "Graphics/Enums.h"
#include <vulkan/vulkan_core.h>

using namespace Seele;
using namespace Seele::Vulkan;

struct PendingBuffer {
    OBufferAllocation allocation;
    uint64 offset;
    Gfx::QueueType prevQueue;
    bool writeOnly;
};

static Map<BufferAllocation*, PendingBuffer> pendingBuffers;

BufferAllocation::BufferAllocation(PGraphics graphics, VkBufferCreateInfo bufferInfo, VmaAllocationCreateInfo allocInfo,
                                   Gfx::QueueType owner)
    : CommandBoundResource(graphics), size(bufferInfo.size), owner(owner) {
    VK_CHECK(vmaCreateBuffer(graphics->getAllocator(), &bufferInfo, &allocInfo, &buffer, &allocation, nullptr));
    vmaGetAllocationMemoryProperties(graphics->getAllocator(), allocation, &properties);
    VkBufferDeviceAddressInfo addrInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
        .pNext = nullptr,
        .buffer = buffer,
    };
    deviceAddress = vkGetBufferDeviceAddress(graphics->getDevice(), &addrInfo);
}

BufferAllocation::~BufferAllocation() {
    if (buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(graphics->getAllocator(), buffer, allocation);
    }
}

void BufferAllocation::pipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                       VkPipelineStageFlags dstStage) {
    if (size == 0)
        return;
    PCommand commandBuffer = graphics->getQueueCommands(owner)->getCommands();
    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = srcAccess,
        .dstAccessMask = dstAccess,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
        .offset = 0,
        .size = size,
    };
    vkCmdPipelineBarrier(commandBuffer->getHandle(), srcStage, dstStage, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

void BufferAllocation::transferOwnership(Gfx::QueueType newOwner) {
    if (size == 0)
        return;
    Gfx::QueueFamilyMapping mapping = graphics->getFamilyMapping();
    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        .srcQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(owner),
        .dstQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(newOwner),
        .buffer = buffer,
        .offset = 0,
        .size = size,
    };
    PCommandPool sourcePool = graphics->getQueueCommands(owner);
    PCommandPool dstPool = graphics->getQueueCommands(newOwner);
    assert(barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex);
    vkCmdPipelineBarrier(sourcePool->getCommands()->getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                         0, nullptr, 1, &barrier, 0, nullptr);
    vkCmdPipelineBarrier(dstPool->getCommands()->getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                         nullptr, 1, &barrier, 0, nullptr);
    sourcePool->submitCommands();
    owner = newOwner;
}

void* BufferAllocation::mapRegion(uint64 regionOffset, uint64 regionSize, bool writeOnly) {
    void* data = nullptr;
    PendingBuffer pending;
    pending.writeOnly = writeOnly;
    pending.prevQueue = owner;
    pending.offset = regionOffset;
    transferOwnership(Gfx::QueueType::TRANSFER);
    if (writeOnly) {
        if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            VK_CHECK(vmaMapMemory(graphics->getAllocator(), allocation, &data));
        } else {
            VkBufferCreateInfo stagingInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .size = regionSize,
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };
            VmaAllocationCreateInfo allocInfo = {
                .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO,
                .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            };
            pending.allocation = new BufferAllocation(graphics, stagingInfo, allocInfo, Gfx::QueueType::TRANSFER);
            vmaMapMemory(graphics->getAllocator(), pending.allocation->allocation, &data);
            vmaSetAllocationName(graphics->getAllocator(), pending.allocation->allocation, "MappingStaging");
        }
    } else {
        assert(false);
    }
    pendingBuffers[this] = std::move(pending);
    return data;
}

void BufferAllocation::unmap() {
    auto found = pendingBuffers.find(this);
    if (found != pendingBuffers.end()) {
        PendingBuffer& pending = found->value;
        if (pending.writeOnly) {
            if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                vmaUnmapMemory(graphics->getAllocator(), allocation);
            } else {
                vmaFlushAllocation(graphics->getAllocator(), pending.allocation->allocation, 0, VK_WHOLE_SIZE);
                vmaUnmapMemory(graphics->getAllocator(), pending.allocation->allocation);
                PCommand command = graphics->getQueueCommands(owner)->getCommands();
                command->bindResource(PBufferAllocation(pending.allocation));
                VkCommandBuffer cmdHandle = command->getHandle();

                VkBufferCopy region = {
                    .srcOffset = 0,
                    .dstOffset = pending.offset,
                    .size = pending.allocation->size,
                };
                VkBufferMemoryBarrier barrier = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = buffer,
                    .offset = 0,
                    .size = size,
                };
                vkCmdPipelineBarrier(cmdHandle, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                                     &barrier, 0, nullptr);
                vkCmdCopyBuffer(cmdHandle, pending.allocation->buffer, buffer, 1, &region);
                barrier = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = buffer,
                    .offset = 0,
                    .size = size,
                };
                vkCmdPipelineBarrier(cmdHandle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1,
                                     &barrier, 0, nullptr);
                graphics->getDestructionManager()->queueResourceForDestruction(std::move(pending.allocation));
            }
        }
        transferOwnership(pending.prevQueue);
        pendingBuffers.erase(this);
    }
}

Buffer::Buffer(PGraphics graphics, uint64 size, VkBufferUsageFlags usage, Gfx::QueueType queueType, bool dynamic, std::string name)
    : graphics(graphics), currentBuffer(0), initialOwner(queueType),
      usage(usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT),
      dynamic(dynamic), name(name) {
    createBuffer(size);
}

Buffer::~Buffer() {
    for (uint32 i = 0; i < buffers.size(); ++i) {
        graphics->getDestructionManager()->queueResourceForDestruction(std::move(buffers[i]));
    }
}

void* Buffer::map(bool writeOnly) { return mapRegion(0, getSize(), writeOnly); }

void* Buffer::mapRegion(uint64 regionOffset, uint64 regionSize, bool writeOnly) {
    if (regionSize == 0)
        return nullptr;
    void* data = getAlloc()->mapRegion(regionOffset, regionSize, writeOnly);
    assert(data);
    return data;
}

void Buffer::unmap() { getAlloc()->unmap(); }

void Buffer::rotateBuffer(uint64 size, bool preserveContents) {
    assert(dynamic);
    size = std::max(getSize(), size);
    for (uint32 i = 0; i < buffers.size(); ++i) {
        if (buffers[i]->isCurrentlyBound()) {
            continue;
        }
        if (buffers[i]->size < size) {
            vmaDestroyBuffer(graphics->getAllocator(), buffers[i]->buffer, buffers[i]->allocation);
            uint32 family = graphics->getFamilyMapping().getQueueTypeFamilyIndex(initialOwner);
            VkBufferCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .size = size,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 1,
                .pQueueFamilyIndices = &family,
            };
            VmaAllocationCreateInfo allocInfo = {
                .flags =
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                .usage = VMA_MEMORY_USAGE_AUTO,
            };
            VK_CHECK(vmaCreateBuffer(graphics->getAllocator(), &info, &allocInfo, &buffers[i]->buffer, &buffers[i]->allocation,
                                     &buffers[i]->info));
            vmaGetAllocationMemoryProperties(graphics->getAllocator(), buffers[i]->allocation, &buffers[i]->properties);
            if (!name.empty()) {
                VkDebugUtilsObjectNameInfoEXT nameInfo = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                                                          .pNext = nullptr,
                                                          .objectType = VK_OBJECT_TYPE_BUFFER,
                                                          .objectHandle = (uint64)buffers[i]->buffer,
                                                          .pObjectName = this->name.c_str()};
                vkSetDebugUtilsObjectNameEXT(graphics->getDevice(), &nameInfo);
            }
            buffers[i]->size = size;
        }
        if (preserveContents) {
            copyBuffer(currentBuffer, i);
        }
        currentBuffer = i;
        return;
    }
    createBuffer(size);
    if (preserveContents) {
        copyBuffer(currentBuffer, buffers.size() - 1);
    }
    currentBuffer = buffers.size() - 1;
}

void Buffer::createBuffer(uint64 size) {
    uint32 family = graphics->getFamilyMapping().getQueueTypeFamilyIndex(initialOwner);
    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &family,
    };
    VmaAllocationCreateInfo allocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    buffers.add(new BufferAllocation(graphics, info, allocInfo, initialOwner));
    if (size > 0) {
        VK_CHECK(vmaCreateBuffer(graphics->getAllocator(), &info, &allocInfo, &buffers.back()->buffer, &buffers.back()->allocation,
                                 &buffers.back()->info));
        if (!name.empty()) {
            VkDebugUtilsObjectNameInfoEXT nameInfo = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                                                      .pNext = nullptr,
                                                      .objectType = VK_OBJECT_TYPE_BUFFER,
                                                      .objectHandle = (uint64)buffers.back()->buffer,
                                                      .pObjectName = this->name.c_str()};
            vkSetDebugUtilsObjectNameEXT(graphics->getDevice(), &nameInfo);
        }
    }
}

void Buffer::copyBuffer(uint64 src, uint64 dst) {
    if (src == dst) {
        return;
    }
    PCommand command = graphics->getQueueCommands(getAlloc()->owner)->getCommands();
    VkBufferMemoryBarrier srcBarrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffers[src]->buffer,
        .offset = 0,
        .size = buffers[src]->size,
    };
    vkCmdPipelineBarrier(command->getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1,
                         &srcBarrier, 0, nullptr);
    VkBufferCopy region = {.srcOffset = 0, .dstOffset = 0, .size = buffers[src]->size};
    vkCmdCopyBuffer(command->getHandle(), buffers[src]->buffer, buffers[dst]->buffer, 1, &region);
    VkBufferMemoryBarrier dstBarrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffers[dst]->buffer,
        .offset = 0,
        .size = buffers[dst]->size,
    };
    vkCmdPipelineBarrier(command->getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 1,
                         &dstBarrier, 0, nullptr);
}

void Buffer::transferOwnership(Gfx::QueueType newOwner) { getAlloc()->transferOwnership(newOwner); }

void Buffer::pipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                             VkPipelineStageFlags dstStage) {
    getAlloc()->pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

UniformBuffer::UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo& createInfo)
    : Gfx::UniformBuffer(graphics->getFamilyMapping(), createInfo),
      Vulkan::Buffer(graphics, createInfo.sourceData.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, createInfo.sourceData.owner,
                     createInfo.dynamic, createInfo.name) {
    if (getSize() > 0 && createInfo.sourceData.data != nullptr) {
        void* data = map();
        std::memcpy(data, createInfo.sourceData.data, createInfo.sourceData.size);
        unmap();
    }
}

UniformBuffer::~UniformBuffer() {}

void UniformBuffer::updateContents(const DataSource& sourceData) {
    void* data = map();
    std::memcpy(data, sourceData.data, sourceData.size);
    unmap();
}

void UniformBuffer::rotateBuffer(uint64 size) { Vulkan::Buffer::rotateBuffer(size); }

void UniformBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Vulkan::Buffer::transferOwnership(newOwner); }

void UniformBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                           VkPipelineStageFlags dstStage) {
    Vulkan::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

ShaderBuffer::ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo& createInfo)
    : Gfx::ShaderBuffer(graphics->getFamilyMapping(), createInfo),
      Vulkan::Buffer(graphics, createInfo.sourceData.size,
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                         (createInfo.vertexBuffer ? VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                                                  : 0),
                     createInfo.sourceData.owner, createInfo.dynamic, createInfo.name) {
    if (getSize() > 0 && createInfo.sourceData.data != nullptr) {
        void* data = map();
        std::memcpy(data, createInfo.sourceData.data, createInfo.sourceData.size);
        unmap();
    }
}

ShaderBuffer::~ShaderBuffer() {}

void ShaderBuffer::updateContents(const ShaderBufferCreateInfo& createInfo) {
    if (createInfo.sourceData.data == nullptr) {
        return;
    }
    // We always want to update, as the contents could be different on the GPU
    void* data = map();
    std::memcpy((char*)data + createInfo.sourceData.offset, createInfo.sourceData.data, createInfo.sourceData.size);
    unmap();
}

void Seele::Vulkan::ShaderBuffer::rotateBuffer(uint64 size, bool preserveContents) {
    assert(dynamic);
    Vulkan::Buffer::rotateBuffer(size, preserveContents);
}

void* ShaderBuffer::mapRegion(uint64 offset, uint64 size, bool writeOnly) { return Vulkan::Buffer::mapRegion(offset, size, writeOnly); }

void ShaderBuffer::unmap() { Vulkan::Buffer::unmap(); }

void ShaderBuffer::clear() {
    vkCmdFillBuffer(graphics->getQueueCommands(getAlloc()->owner)->getCommands()->getHandle(), Vulkan::Buffer::getHandle(), 0,
                    VK_WHOLE_SIZE, 0);
}

void ShaderBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Vulkan::Buffer::transferOwnership(newOwner); }

void ShaderBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                          VkPipelineStageFlags dstStage) {
    Vulkan::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

VertexBuffer::VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo& createInfo)
    : Gfx::VertexBuffer(graphics->getFamilyMapping(), createInfo),
      Vulkan::Buffer(graphics, createInfo.sourceData.size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, createInfo.sourceData.owner, false,
                     createInfo.name) {
    if (createInfo.sourceData.data != nullptr) {
        void* data = map();
        std::memcpy(data, createInfo.sourceData.data, createInfo.sourceData.size);
        unmap();
    }
}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::updateRegion(DataSource update) {
    void* data = mapRegion(update.offset, update.size);
    std::memcpy(data, update.data, update.size);
    unmap();
}

void VertexBuffer::download(Array<uint8>& buffer) {
    void* data = map(false);
    buffer.resize(getSize());
    std::memcpy(buffer.data(), data, getSize());
    unmap();
}

void VertexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Vulkan::Buffer::transferOwnership(newOwner); }

void VertexBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                          VkPipelineStageFlags dstStage) {
    Vulkan::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

IndexBuffer::IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo& createInfo)
    : Gfx::IndexBuffer(graphics->getFamilyMapping(), createInfo),
      Vulkan::Buffer(graphics, createInfo.sourceData.size,
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                         VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                     createInfo.sourceData.owner, false, createInfo.name) {
    if (createInfo.sourceData.data != nullptr) {
        void* data = map();
        std::memcpy(data, createInfo.sourceData.data, createInfo.sourceData.size);
        unmap();
    }
}

IndexBuffer::~IndexBuffer() {}

void IndexBuffer::download(Array<uint8>& buffer) {
    void* data = map(false);
    buffer.resize(getSize());
    std::memcpy(buffer.data(), data, getSize());
    unmap();
}

void IndexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Vulkan::Buffer::transferOwnership(newOwner); }

void IndexBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                         VkPipelineStageFlags dstStage) {
    Vulkan::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
