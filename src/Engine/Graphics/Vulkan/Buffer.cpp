#include "Buffer.h"
#include "Command.h"
#include "Enums.h"
#include "Graphics/Enums.h"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

using namespace Seele;
using namespace Seele::Vulkan;

BufferAllocation::BufferAllocation(PGraphics graphics, const std::string& name, VkBufferCreateInfo bufferInfo,
                                   VmaAllocationCreateInfo allocInfo, Gfx::QueueType owner, uint64 alignment)
    : CommandBoundResource(graphics), size(bufferInfo.size), name(name), owner(owner) {
    VK_CHECK(vmaCreateBufferWithAlignment(graphics->getAllocator(), &bufferInfo, &allocInfo, alignment, &buffer, &allocation, &info));
    vmaGetAllocationMemoryProperties(graphics->getAllocator(), allocation, &properties);
    VkDebugUtilsObjectNameInfoEXT nameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_BUFFER,
        .objectHandle = (uint64)buffer,
        .pObjectName = name.c_str(),
    };
    vkSetDebugUtilsObjectNameEXT(graphics->getDevice(), &nameInfo);
    if (bufferInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo addrInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR,
            .pNext = nullptr,
            .buffer = buffer,
        };
        deviceAddress = vkGetBufferDeviceAddress(graphics->getDevice(), &addrInfo);
    }
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
    if (mapping.getQueueTypeFamilyIndex(newOwner) == mapping.getQueueTypeFamilyIndex(owner))
        return;
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

void BufferAllocation::updateContents(uint64 regionOffset, uint64 regionSize, void* ptr) {
    if (regionSize == 0)
        return;
    VkBufferCreateInfo stagingInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = regionSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    VmaAllocationCreateInfo stagingAlloc = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    OBufferAllocation staging = new BufferAllocation(graphics, "UpdateStaging", stagingInfo, stagingAlloc, Gfx::QueueType::TRANSFER);

    uint8* data;
    VK_CHECK(vmaMapMemory(graphics->getAllocator(), staging->allocation, (void**)&data));
    std::memcpy(data, ptr, regionSize);
    VK_CHECK(vmaFlushAllocation(graphics->getAllocator(), staging->allocation, 0, regionSize));
    vmaUnmapMemory(graphics->getAllocator(), staging->allocation);

    Gfx::QueueType prevOwner = owner;
    transferOwnership(Gfx::QueueType::TRANSFER);

    PCommand cmd = graphics->getQueueCommands(Gfx::QueueType::TRANSFER)->getCommands();
    VkBufferCopy copy = {
        .srcOffset = 0,
        .dstOffset = regionOffset,
        .size = regionSize,
    };
    vkCmdCopyBuffer(cmd->getHandle(), staging->buffer, buffer, 1, &copy);
    cmd->bindResource(PBufferAllocation(this));
    cmd->bindResource(PBufferAllocation(staging));

    transferOwnership(prevOwner);
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(staging));
}

void BufferAllocation::readContents(uint64 regionOffset, uint64 regionSize, void* ptr) {
    if (regionSize == 0)
        return;

    VkBufferCreateInfo stagingInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = regionSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    VmaAllocationCreateInfo stagingAlloc = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    OBufferAllocation staging = new BufferAllocation(graphics, "ReadStaging", stagingInfo, stagingAlloc, Gfx::QueueType::TRANSFER);

    Gfx::QueueType prevOwner = owner;
    transferOwnership(Gfx::QueueType::TRANSFER);

    PCommandPool pool = graphics->getQueueCommands(Gfx::QueueType::TRANSFER);
    PCommand cmd = pool->getCommands();
    VkBufferCopy copy = {
        .srcOffset = regionOffset,
        .dstOffset = 0,
        .size = regionSize,
    };
    vkCmdCopyBuffer(cmd->getHandle(), buffer, staging->buffer, 1, &copy);
    cmd->bindResource(PBufferAllocation(this));
    cmd->bindResource(PBufferAllocation(staging));
    pool->submitCommands();
    cmd->getFence()->wait(1000000);

    transferOwnership(prevOwner);

    uint8* data;
    VK_CHECK(vmaMapMemory(graphics->getAllocator(), staging->allocation, (void**)&data));
    std::memcpy(ptr, data + regionOffset, regionSize);
    VK_CHECK(vmaFlushAllocation(graphics->getAllocator(), staging->allocation, regionOffset, regionSize));
    vmaUnmapMemory(graphics->getAllocator(), staging->allocation);
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(staging));
}

Buffer::Buffer(PGraphics graphics, uint64 size, VkBufferUsageFlags usage, Gfx::QueueType queueType, bool dynamic, std::string name,
               bool createCleared, uint32 clearValue)
    : graphics(graphics), currentBuffer(0), initialOwner(queueType),
      usage(usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT),
      dynamic(dynamic), createCleared(createCleared), name(name), clearValue(clearValue) {
    if (size > 0) {
        buffers.add(nullptr);
        createBuffer(size, 0);
    }
}

Buffer::~Buffer() {
    for (uint32 i = 0; i < buffers.size(); ++i) {
        graphics->getDestructionManager()->queueResourceForDestruction(std::move(buffers[i]));
    }
}

void Buffer::updateContents(uint64 regionOffset, uint64 regionSize, void* buffer) {
    getAlloc()->updateContents(regionOffset, regionSize, buffer);
}

void Buffer::readContents(uint64 regionOffset, uint64 regionSize, void* buffer) {
    getAlloc()->readContents(regionOffset, regionSize, buffer);
}

void Buffer::rotateBuffer(uint64 size, bool preserveContents) {
    if (size == 0)
        return;
    assert(dynamic);
    for (uint32 i = 0; i < buffers.size(); ++i) {
        if (buffers[i]->isCurrentlyBound()) {
            continue;
        }
        if (buffers[i]->size < size) {
            graphics->getDestructionManager()->queueResourceForDestruction(std::move(buffers[i]));
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
    if (size > 0) {
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
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        };
        buffers[destIndex] = new BufferAllocation(graphics, name, info, allocInfo, initialOwner);
        if (createCleared)
        {
            PCommand command = graphics->getQueueCommands(initialOwner)->getCommands();
            vkCmdFillBuffer(command->getHandle(), buffers[destIndex]->buffer, 0, VK_WHOLE_SIZE, clearValue);
            pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
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
    VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = buffers[src]->size,
    };
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
    if (createInfo.sourceData.size > 0 && createInfo.sourceData.data != nullptr) {
        getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    }
}

UniformBuffer::~UniformBuffer() {}

void UniformBuffer::updateContents(const DataSource& sourceData) {
    if (sourceData.size > 0 && sourceData.data != nullptr) {
        getAlloc()->updateContents(sourceData.offset, sourceData.size, sourceData.data);
    }
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
                     createInfo.sourceData.owner, createInfo.dynamic, createInfo.name, createInfo.createCleared, createInfo.clearValue) {
    if (createInfo.sourceData.size > 0 && createInfo.sourceData.data != nullptr) {
        getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    }
}

ShaderBuffer::~ShaderBuffer() {}

void ShaderBuffer::updateContents(const ShaderBufferCreateInfo& createInfo) {
    if (createInfo.sourceData.data == nullptr) {
        return;
    }
    // We always want to update, as the contents could be different on the GPU
    if (createInfo.sourceData.size > 0 && createInfo.sourceData.data != nullptr) {
        getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    }
}

void ShaderBuffer::rotateBuffer(uint64 size, bool preserveContents) {
    assert(dynamic);
    Vulkan::Buffer::rotateBuffer(size, preserveContents);
}

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
    if (createInfo.sourceData.size > 0 && createInfo.sourceData.data != nullptr) {
        getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    }
}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::updateRegion(DataSource sourceData) { getAlloc()->updateContents(sourceData.offset, sourceData.size, sourceData.data); }

void VertexBuffer::download(Array<uint8>& buffer) {
    buffer.resize(getSize());
    getAlloc()->readContents(0, buffer.size(), buffer.data());
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
                         VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     createInfo.sourceData.owner, false, createInfo.name) {
    getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
}

IndexBuffer::~IndexBuffer() {}

void IndexBuffer::download(Array<uint8>& buffer) {
    buffer.resize(getSize());
    getAlloc()->readContents(0, buffer.size(), buffer.data());
}

void IndexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner) { Vulkan::Buffer::transferOwnership(newOwner); }

void IndexBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                         VkPipelineStageFlags dstStage) {
    Vulkan::Buffer::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
