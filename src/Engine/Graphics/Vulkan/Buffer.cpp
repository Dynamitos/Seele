#include "Buffer.h"
#include "Command.h"
#include "Enums.h"
#include "Graphics/Enums.h"
#include <fmt/format.h>
#include <vk_mem_alloc.h>

using namespace Seele;
using namespace Seele::Vulkan;

BufferAllocation::BufferAllocation(PGraphics graphics, const std::string& name, VkBufferCreateInfo bufferInfo,
                                   VmaAllocationCreateInfo allocInfo, Gfx::QueueType owner, uint64 alignment)
    : CommandBoundResource(graphics, name), size(bufferInfo.size), owner(owner) {
    if (bufferInfo.size == 0)
        return;
    VK_CHECK(vmaCreateBufferWithAlignment(graphics->getAllocator(), &bufferInfo, &allocInfo, alignment, &buffer, &allocation, &info));
    vmaGetAllocationMemoryProperties(graphics->getAllocator(), allocation, &properties);
    assert(!name.empty());
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
    commandBuffer->bindResource(this);
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
    sourcePool->getCommands()->bindResource(this);
    dstPool->getCommands()->bindResource(this);
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
    OBufferAllocation staging =
        new BufferAllocation(graphics, fmt::format("{0}UpdateStaging", name), stagingInfo, stagingAlloc, Gfx::QueueType::GRAPHICS);

    uint8* data;
    VK_CHECK(vmaMapMemory(graphics->getAllocator(), staging->allocation, (void**)&data));
    std::memcpy(data, ptr, regionSize);
    VK_CHECK(vmaFlushAllocation(graphics->getAllocator(), staging->allocation, 0, regionSize));
    vmaUnmapMemory(graphics->getAllocator(), staging->allocation);

    Gfx::QueueType prevOwner = owner;
    transferOwnership(Gfx::QueueType::TRANSFER);

    PCommand cmd = graphics->getQueueCommands(Gfx::QueueType::GRAPHICS)->getCommands();
    VkBufferCopy copy = {
        .srcOffset = 0,
        .dstOffset = regionOffset,
        .size = regionSize,
    };
    cmd->bindResource(PBufferAllocation(this));
    cmd->bindResource(PBufferAllocation(staging));
    vkCmdCopyBuffer(cmd->getHandle(), staging->buffer, buffer, 1, &copy);
    pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT);

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
    OBufferAllocation staging = new BufferAllocation(graphics, "ReadStaging", stagingInfo, stagingAlloc, Gfx::QueueType::GRAPHICS);

    Gfx::QueueType prevOwner = owner;
    transferOwnership(Gfx::QueueType::TRANSFER);

    PCommandPool pool = graphics->getQueueCommands(Gfx::QueueType::TRANSFER);
    PCommand cmd = pool->getCommands();
    VkBufferCopy copy = {
        .srcOffset = regionOffset,
        .dstOffset = 0,
        .size = regionSize,
    };
    cmd->bindResource(PBufferAllocation(this));
    cmd->bindResource(PBufferAllocation(staging));
    vkCmdCopyBuffer(cmd->getHandle(), buffer, staging->buffer, 1, &copy);
    pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT);
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

void* BufferAllocation::map() {
    if (mappedPointer == nullptr) {
        vmaMapMemory(graphics->getAllocator(), allocation, &mappedPointer);
    }
    return mappedPointer;
}

void BufferAllocation::unmap() {
    vmaUnmapMemory(graphics->getAllocator(), allocation);
    mappedPointer = nullptr;
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
    if (buffers.size() == 0)
        return;
    getAlloc()->updateContents(regionOffset, regionSize, buffer);
}

void Buffer::readContents(uint64 regionOffset, uint64 regionSize, void* buffer) {
    if (buffers.size() == 0)
        return;
    getAlloc()->readContents(regionOffset, regionSize, buffer);
}

void* Buffer::map() {
    if (stagingBuffer == nullptr) {
        stagingBuffer = new BufferAllocation(graphics, fmt::format("{}Staging", name),
                                             VkBufferCreateInfo{
                                                 .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                                 .pNext = nullptr,
                                                 .flags = 0,
                                                 .size = getSize(),
                                                 .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                             },
                                             VmaAllocationCreateInfo{
                                                 .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                                                 .usage = VMA_MEMORY_USAGE_AUTO,
                                             },
                                             Gfx::QueueType::GRAPHICS);
    }
    return stagingBuffer->map();
}

void Buffer::unmap() {
    if (stagingBuffer == nullptr)
        return;
    stagingBuffer->unmap();

    PCommand cmd = graphics->getQueueCommands(Gfx::QueueType::GRAPHICS)->getCommands();
    VkBufferCopy copy = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = getSize(),
    };
    cmd->bindResource(getAlloc());
    cmd->bindResource(PBufferAllocation(stagingBuffer));
    vkCmdCopyBuffer(cmd->getHandle(), stagingBuffer->buffer, getAlloc()->buffer, 1, &copy);
    pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT);

    // transferOwnership(prevOwner);
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(stagingBuffer));
    stagingBuffer = nullptr;
}

void Buffer::rotateBuffer(uint64 size, bool preserveContents) {
    if (size == 0)
        return;
    assert(dynamic);
    unmap();
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
    createBuffer(size, (uint32)buffers.size() - 1);
    if (preserveContents) {
        copyBuffer(currentBuffer, buffers.size() - 1);
    }
    currentBuffer = (uint32)buffers.size() - 1;
}

void Buffer::createBuffer(uint64 size, uint32 destIndex) {
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
    if (createCleared) {
        PCommand command = graphics->getQueueCommands(initialOwner)->getCommands();
        command->bindResource(PBufferAllocation(buffers[destIndex]));
        vkCmdFillBuffer(command->getHandle(), buffers[destIndex]->buffer, 0, VK_WHOLE_SIZE, clearValue);
        buffers[destIndex]->pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                            VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
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
    VkBufferMemoryBarrier clearBarrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffers[dst]->buffer,
        .offset = 0,
        .size = buffers[dst]->size,
    };
    VkBufferMemoryBarrier srcBarriers[] = {srcBarrier, clearBarrier};
    command->bindResource(PBufferAllocation(buffers[src]));
    command->bindResource(PBufferAllocation(buffers[dst]));
    vkCmdPipelineBarrier(command->getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 2,
                         srcBarriers, 0, nullptr);
    VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = buffers[src]->size,
    };
    command->bindResource(PBufferAllocation(buffers[src]));
    command->bindResource(PBufferAllocation(buffers[dst]));
    vkCmdCopyBuffer(command->getHandle(), buffers[src]->buffer, buffers[dst]->buffer, 1, &region);
    VkBufferMemoryBarrier dstBarrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffers[dst]->buffer,
        .offset = 0,
        .size = buffers[dst]->size,
    };
    command->bindResource(PBufferAllocation(buffers[dst]));
    vkCmdPipelineBarrier(command->getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &dstBarrier, 0, nullptr);
}

void Buffer::transferOwnership(Gfx::QueueType newOwner) {
    if (buffers.size() == 0)
        return;
    getAlloc()->transferOwnership(newOwner);
}

void Buffer::pipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                             VkPipelineStageFlags dstStage) {
    if (buffers.size() == 0)
        return;
    getAlloc()->pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

UniformBuffer::UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo& createInfo)
    : Gfx::UniformBuffer(graphics->getFamilyMapping(), createInfo),
      Vulkan::Buffer(graphics, createInfo.sourceData.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, createInfo.sourceData.owner, true,
                     createInfo.name) {
    if (createInfo.sourceData.size > 0 && createInfo.sourceData.data != nullptr) {
        getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    }
}

UniformBuffer::~UniformBuffer() {}

void UniformBuffer::updateContents(uint64 offset, uint64 size, void* data) {
    if (size > 0 && data != nullptr) {
        rotateBuffer(size);
        getAlloc()->updateContents(offset, size, data);
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
      Vulkan::Buffer(graphics, createInfo.sourceData.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | createInfo.usage,
                     createInfo.sourceData.owner, true, createInfo.name, createInfo.sourceData.data == nullptr, createInfo.clearValue) {
    if (createInfo.sourceData.size > 0 && createInfo.sourceData.data != nullptr) {
        getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    }
}

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
    Vulkan::Buffer::rotateBuffer(size, preserveContents);
}

void* ShaderBuffer::map() { return Vulkan::Buffer::map(); }

void ShaderBuffer::unmap() { Vulkan::Buffer::unmap(); }

void ShaderBuffer::clear() {
    if (getAlloc() == nullptr)
        return;
    PCommand command = graphics->getQueueCommands(getAlloc()->owner)->getCommands();
    command->bindResource(PBufferAllocation(getAlloc()));
    vkCmdFillBuffer(command->getHandle(), Vulkan::Buffer::getHandle(), 0, VK_WHOLE_SIZE, 0);
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

void VertexBuffer::updateRegion(uint64 offset, uint64 size, void* data) { getAlloc()->updateContents(offset, size, data); }

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
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                     createInfo.sourceData.owner, false, createInfo.name) {
    getAlloc()->updateContents(createInfo.sourceData.offset, createInfo.sourceData.size, createInfo.sourceData.data);
    // getAlloc()->pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_INDEX_READ_BIT,
    //                             Gfx::SE_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | Gfx::SE_PIPELINE_STAGE_VERTEX_INPUT_BIT);
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
