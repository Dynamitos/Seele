#include "Buffer.h"
#include "Command.h"

using namespace Seele;
using namespace Seele::Vulkan;

struct PendingBuffer
{
    uint64 offset;
    uint64 size;
    OStagingBuffer stagingBuffer;
    Gfx::QueueType prevQueue;
    bool writeOnly;
};

static std::map<Vulkan::Buffer*, PendingBuffer> pendingBuffers;

Buffer::Buffer(PGraphics graphics, uint64 size, VkBufferUsageFlags usage, Gfx::QueueType& queueType, bool dynamic)
    : graphics(graphics)
    , currentBuffer(0)
    , size(size)
    , owner(queueType)
{
    if(dynamic)
    {
        numBuffers = Gfx::numFramesBuffered;
    } 
    else
    {
        numBuffers = 1;
    }
    usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    uint32 queueFamilyIndex =  graphics->getFamilyMapping().getQueueTypeFamilyIndex(queueType);
    VkBufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &queueFamilyIndex,
    };
    VkBufferMemoryRequirementsInfo2 bufferReqInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
        .pNext = nullptr,
    };
    VkMemoryDedicatedRequirements dedicatedRequirements = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS,
        .pNext = nullptr,
    };
    VkMemoryRequirements2 memRequirements = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = &dedicatedRequirements,
    };
    for (uint32 i = 0; i < numBuffers; ++i)
    {
        VK_CHECK(vkCreateBuffer(graphics->getDevice(), &info, nullptr, &buffers[i].buffer));
        bufferReqInfo.buffer = buffers[i].buffer;
        vkGetBufferMemoryRequirements2(graphics->getDevice(), &bufferReqInfo, &memRequirements);
        buffers[i].allocation = graphics->getAllocator()->allocate(memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffers[i].buffer);
        vkBindBufferMemory(graphics->getDevice(), buffers[i].buffer, buffers[i].allocation->getHandle(), buffers[i].allocation->getOffset());
    }
}

Buffer::~Buffer()
{
    for(uint32 i = 0; i < numBuffers; ++i)
    {        
        graphics->getDestructionManager()->queueBuffer(graphics->getQueueCommands(owner)->getCommands(), buffers[i].buffer);
    }
}

VkDeviceSize Buffer::getOffset() const
{
    return buffers[currentBuffer].allocation->getOffset();
}

void Buffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    Gfx::QueueFamilyMapping mapping = graphics->getFamilyMapping();
    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(owner),
        .dstQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(newOwner),
        .offset = 0,
        .size = size,
    };
    PCommandPool sourcePool = graphics->getQueueCommands(owner);
    PCommandPool dstPool = nullptr;
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    assert(barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex);
    if (owner == Gfx::QueueType::TRANSFER || owner == Gfx::QueueType::DEDICATED_TRANSFER)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (owner == Gfx::QueueType::COMPUTE)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if (owner == Gfx::QueueType::GRAPHICS)
    {
        barrier.srcAccessMask = getSourceAccessMask();
        srcStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }
    if (newOwner == Gfx::QueueType::TRANSFER)
    {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstPool = graphics->getTransferCommands();
    }
    else if (newOwner == Gfx::QueueType::DEDICATED_TRANSFER)
    {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstPool = graphics->getDedicatedTransferCommands();
    }
    else if (newOwner == Gfx::QueueType::COMPUTE)
    {
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dstPool = graphics->getComputeCommands();
    }
    else if (newOwner == Gfx::QueueType::GRAPHICS)
    {
        barrier.dstAccessMask = getDestAccessMask();
        dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        dstPool = graphics->getGraphicsCommands();
    }
    VkCommandBuffer srcCommand = sourcePool->getCommands()->getHandle();
    VkCommandBuffer dstCommand = dstPool->getCommands()->getHandle();
    VkBufferMemoryBarrier dynamicBarriers[Gfx::numFramesBuffered];
    for (uint32 i = 0; i < numBuffers; ++i)
    {
        dynamicBarriers[i] = barrier;
        dynamicBarriers[i].buffer = buffers[i].buffer;
    }
    vkCmdPipelineBarrier(srcCommand, srcStage, srcStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
    vkCmdPipelineBarrier(dstCommand, dstStage, dstStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
    sourcePool->submitCommands();
}

void Buffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    PCommand commandBuffer = graphics->getQueueCommands(owner)->getCommands();
    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr, 
        .srcAccessMask = srcAccess,
        .dstAccessMask = dstAccess,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .offset = 0,
        .size = size,
    };
    VkBufferMemoryBarrier dynamicBarriers[Gfx::numFramesBuffered];
    for(uint32 i = 0; i < numBuffers; ++i)
    {
        dynamicBarriers[i] = barrier;
        dynamicBarriers[i].buffer = buffers[i].buffer;
    }
    vkCmdPipelineBarrier(commandBuffer->getHandle(), srcStage, dstStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
}

void * Buffer::map(bool writeOnly)
{
    return mapRegion(0, size, writeOnly);
}

void * Buffer::mapRegion(uint64 regionOffset, uint64 regionSize, bool writeOnly)
{
    void *data = nullptr;

    /*if (bVolatile)
    {
        if (lockMode == RLM_ReadOnly)
        {
            assert(0);
        }
        else
        {
            throw new std::logic_error("TODO implement volatile buffers");
            //device->getRHIDevice()->getTemp
        }
    }*/
    //assert(bStatic || bDynamic || bUAV);

    PendingBuffer pending;
    pending.writeOnly = writeOnly;
    pending.prevQueue = owner;
    pending.offset = regionOffset;
    pending.size = regionSize;
    if (writeOnly)
    {
        //requestOwnershipTransfer(Gfx::QueueType::DEDICATED_TRANSFER);
        OStagingBuffer stagingBuffer = graphics->getStagingManager()->create(regionSize);
        data = stagingBuffer->map();
        pending.stagingBuffer = std::move(stagingBuffer);
    }
    else
    {
        PCommand current = graphics->getQueueCommands(owner)->getCommands();
        current->waitForCommand();

        requestOwnershipTransfer(Gfx::QueueType::DEDICATED_TRANSFER);
        VkCommandBuffer handle = graphics->getQueueCommands(owner)->getCommands()->getHandle();

        VkBufferMemoryBarrier barrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr, 
            .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = buffers[currentBuffer].buffer,
            .offset = 0,
            .size = size,
        };
        vkCmdPipelineBarrier(handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

        OStagingBuffer stagingBuffer = graphics->getStagingManager()->create(size);

        VkBufferCopy regions = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size,
        };
        
        vkCmdCopyBuffer(handle, buffers[currentBuffer].buffer, stagingBuffer->getHandle(), 1, &regions);

        graphics->getQueueCommands(owner)->submitCommands();
        vkQueueWaitIdle(graphics->getQueueCommands(owner)->getQueue()->getHandle());
        stagingBuffer->map(); // this maps the memory if not mapped already
        stagingBuffer->flush();
        data = stagingBuffer->map();

        pending.stagingBuffer = std::move(stagingBuffer);

    }
    pendingBuffers[this] = std::move(pending);

    assert(data);
    return data;
}

void Buffer::unmap()
{
    auto found = pendingBuffers.find(this);
    if (found != pendingBuffers.end())
    {
        PendingBuffer& pending = found->second;
        pending.stagingBuffer->flush();
        if (pending.writeOnly)
        {
            PStagingBuffer stagingBuffer = pending.stagingBuffer;
            PCommand command = graphics->getQueueCommands(owner)->getCommands();
            VkCommandBuffer cmdHandle = command->getHandle();

            VkBufferCopy region = {
                .srcOffset = 0,
                .dstOffset = pending.offset,
                .size = pending.size,
            };
            vkCmdCopyBuffer(cmdHandle, stagingBuffer->getHandle(), buffers[currentBuffer].buffer, 1, &region);
        }
        //requestOwnershipTransfer(pending.prevQueue);
        pendingBuffers.erase(this);
    }
}

UniformBuffer::UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo &createInfo)
    : Gfx::UniformBuffer(graphics->getFamilyMapping(), createInfo.sourceData)
    , Vulkan::Buffer(graphics, createInfo.sourceData.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, currentOwner, createInfo.dynamic)
    , dedicatedStagingBuffer(nullptr)
{
    if(createInfo.dynamic)
    {
        dedicatedStagingBuffer = graphics->getStagingManager()->create(createInfo.sourceData.size);
    }
    if (createInfo.sourceData.data != nullptr)
    {
        void *data = map();
        std::memcpy(data, createInfo.sourceData.data, createInfo.sourceData.size);
        unmap();
    }
}

UniformBuffer::~UniformBuffer()
{
}

bool UniformBuffer::updateContents(const DataSource &sourceData) 
{
    if(!Gfx::UniformBuffer::updateContents(sourceData))
    {
        // no update was performed, skip
        return false;
    }
    void* data = map();
    std::memcpy(data, sourceData.data, sourceData.size);
    unmap();
    return true;
}
void* UniformBuffer::map(bool writeOnly)
{
    if(dedicatedStagingBuffer != nullptr)
    {
        return dedicatedStagingBuffer->map();
    }
    return Vulkan::Buffer::map(writeOnly);
}

void UniformBuffer::unmap()
{
    if(dedicatedStagingBuffer != nullptr)
    {
        dedicatedStagingBuffer->flush();
        PCommand command = graphics->getQueueCommands(currentOwner)->getCommands();
        VkCommandBuffer cmdHandle = command->getHandle();

        VkBufferCopy region;
        std::memset(&region, 0, sizeof(VkBufferCopy));
        region.size = Vulkan::Buffer::size;
        vkCmdCopyBuffer(cmdHandle, dedicatedStagingBuffer->getHandle(), buffers[currentBuffer].buffer, 1, &region);
    }
    else
    {
        Vulkan::Buffer::unmap();
    }
}

void UniformBuffer::requestOwnershipTransfer(Gfx::QueueType newOwner)
{
    Gfx::QueueOwnedResource::transferOwnership(newOwner);
}

void UniformBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    Vulkan::Buffer::executeOwnershipBarrier(newOwner);
}

void UniformBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    Vulkan::Buffer::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

VkAccessFlags UniformBuffer::getSourceAccessMask()
{
    return VK_ACCESS_MEMORY_WRITE_BIT;
}

VkAccessFlags UniformBuffer::getDestAccessMask()
{
    return VK_ACCESS_UNIFORM_READ_BIT;
}

ShaderBuffer::ShaderBuffer(PGraphics graphics, const ShaderBufferCreateInfo &sourceData)
    : Gfx::ShaderBuffer(graphics->getFamilyMapping(), sourceData.numElements, sourceData.sourceData)
    , Vulkan::Buffer(graphics, sourceData.sourceData.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, currentOwner, sourceData.dynamic)
    , dedicatedStagingBuffer(nullptr)
{
    if(sourceData.dynamic)
    {
        dedicatedStagingBuffer = graphics->getStagingManager()->create(sourceData.sourceData.size);
    }
    if (sourceData.sourceData.data != nullptr)
    {
        void *data = map();
        std::memcpy(data, sourceData.sourceData.data, sourceData.sourceData.size);
        unmap();
    }
}

ShaderBuffer::~ShaderBuffer()
{
}

bool ShaderBuffer::updateContents(const DataSource &sourceData) 
{
    assert(sourceData.size <= getSize());
    Gfx::ShaderBuffer::updateContents(sourceData);
    //We always want to update, as the contents could be different on the GPU
    void* data = map();
    std::memcpy(data, sourceData.data, sourceData.size);
    unmap();
    return true;
}
void* ShaderBuffer::map(bool writeOnly)
{
    if(dedicatedStagingBuffer != nullptr)
    {
        return dedicatedStagingBuffer->map();
    }
    return Vulkan::Buffer::map(writeOnly);
}

void ShaderBuffer::unmap()
{
    if(dedicatedStagingBuffer != nullptr)
    {
        dedicatedStagingBuffer->flush();
        PCommand command = graphics->getQueueCommands(currentOwner)->getCommands();
        VkCommandBuffer cmdHandle = command->getHandle();

        VkBufferCopy region = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = Vulkan::Buffer::size,
        };
        vkCmdCopyBuffer(cmdHandle, dedicatedStagingBuffer->getHandle(), buffers[currentBuffer].buffer, 1, &region);
    }
    else
    {
        Vulkan::Buffer::unmap();
    }
}

void ShaderBuffer::requestOwnershipTransfer(Gfx::QueueType newOwner)
{
    Gfx::QueueOwnedResource::transferOwnership(newOwner);
}

void ShaderBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    Vulkan::Buffer::executeOwnershipBarrier(newOwner);
}

void ShaderBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    Vulkan::Buffer::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

VkAccessFlags ShaderBuffer::getSourceAccessMask()
{
    return VK_ACCESS_MEMORY_WRITE_BIT;
}

VkAccessFlags ShaderBuffer::getDestAccessMask()
{
    return VK_ACCESS_MEMORY_READ_BIT;
}

VertexBuffer::VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo &sourceData)
    : Gfx::VertexBuffer(graphics->getFamilyMapping(), sourceData.numVertices, sourceData.vertexSize, sourceData.sourceData.owner)
    , Vulkan::Buffer(graphics, sourceData.sourceData.size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, currentOwner)
{
    if (sourceData.sourceData.data != nullptr)
    {
        void *data = map();
        std::memcpy(data, sourceData.sourceData.data, sourceData.sourceData.size);
        unmap();
    }
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::updateRegion(DataSource update)
{
    void* data = mapRegion(update.offset, update.size);
    std::memcpy(data, update.data, update.size);
    unmap();
}

void VertexBuffer::download(Array<uint8>& buffer)
{
    void* data = map(false);
    buffer.resize(size);
    std::memcpy(buffer.data(), data, size);
    unmap();
}

void VertexBuffer::requestOwnershipTransfer(Gfx::QueueType newOwner)
{
    Gfx::QueueOwnedResource::transferOwnership(newOwner);
}

void VertexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    Vulkan::Buffer::executeOwnershipBarrier(newOwner);
}

void VertexBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    Vulkan::Buffer::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

VkAccessFlags VertexBuffer::getSourceAccessMask()
{
    return VK_ACCESS_MEMORY_WRITE_BIT;
}

VkAccessFlags VertexBuffer::getDestAccessMask()
{
    return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
}

IndexBuffer::IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo &sourceData)
    : Gfx::IndexBuffer(graphics->getFamilyMapping(), sourceData.sourceData.size, sourceData.indexType, sourceData.sourceData.owner)
    , Vulkan::Buffer(graphics, sourceData.sourceData.size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, currentOwner)
{
    if (sourceData.sourceData.data != nullptr)
    {
        void *data = map();
        std::memcpy(data, sourceData.sourceData.data, sourceData.sourceData.size);
        unmap();
    }
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::download(Array<uint8>& buffer)
{
    void* data = map(false);
    buffer.resize(size);
    std::memcpy(buffer.data(), data, size);
    unmap();
}

void IndexBuffer::requestOwnershipTransfer(Gfx::QueueType newOwner)
{
    Gfx::QueueOwnedResource::transferOwnership(newOwner);
}

void IndexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    Vulkan::Buffer::executeOwnershipBarrier(newOwner);
}

void IndexBuffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    Vulkan::Buffer::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

VkAccessFlags IndexBuffer::getSourceAccessMask()
{
    return VK_ACCESS_MEMORY_WRITE_BIT;
}

VkAccessFlags IndexBuffer::getDestAccessMask()
{
    return VK_ACCESS_INDEX_READ_BIT;
}