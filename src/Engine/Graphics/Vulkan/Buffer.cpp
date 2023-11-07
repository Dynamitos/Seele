#include "Buffer.h"
#include "Initializer.h"
#include "CommandBuffer.h"

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
    VkBufferCreateInfo info =
        init::BufferCreateInfo(
            usage,
            size);
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32 queueFamilyIndex =  graphics->getFamilyMapping().getQueueTypeFamilyIndex(queueType);
    info.pQueueFamilyIndices = &queueFamilyIndex;
    info.queueFamilyIndexCount = 1;
    VkBufferMemoryRequirementsInfo2 bufferReqInfo;
    bufferReqInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
    bufferReqInfo.pNext = nullptr;
    VkMemoryDedicatedRequirements dedicatedRequirements;
    dedicatedRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
    dedicatedRequirements.pNext = nullptr;
    VkMemoryRequirements2 memRequirements;
    memRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    memRequirements.pNext = &dedicatedRequirements;
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
}

VkDeviceSize Buffer::getOffset() const
{
    return buffers[currentBuffer].allocation->getOffset();
}

void Buffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    VkBufferMemoryBarrier barrier =
        init::BufferMemoryBarrier();
    PCommandBufferManager sourceManager = graphics->getQueueCommands(owner);
    PCommandBufferManager dstManager = nullptr;
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    Gfx::QueueFamilyMapping mapping = graphics->getFamilyMapping();
    barrier.srcQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(owner);
    barrier.dstQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(newOwner);
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
        dstManager = graphics->getTransferCommands();
    }
    else if (newOwner == Gfx::QueueType::DEDICATED_TRANSFER)
    {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstManager = graphics->getDedicatedTransferCommands();
    }
    else if (newOwner == Gfx::QueueType::COMPUTE)
    {
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dstManager = graphics->getComputeCommands();
    }
    else if (newOwner == Gfx::QueueType::GRAPHICS)
    {
        barrier.dstAccessMask = getDestAccessMask();
        dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        dstManager = graphics->getGraphicsCommands();
    }
    VkCommandBuffer srcCommand = sourceManager->getCommands()->getHandle();
    VkCommandBuffer dstCommand = dstManager->getCommands()->getHandle();
    VkBufferMemoryBarrier dynamicBarriers[Gfx::numFramesBuffered];
    barrier.offset = 0;
    barrier.size = size;
    for (uint32 i = 0; i < numBuffers; ++i)
    {
        dynamicBarriers[i] = barrier;
        dynamicBarriers[i].buffer = buffers[i].buffer;
    }
    vkCmdPipelineBarrier(srcCommand, srcStage, srcStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
    vkCmdPipelineBarrier(dstCommand, dstStage, dstStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
    sourceManager->submitCommands();
}

void Buffer::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    PCmdBuffer commandBuffer = graphics->getQueueCommands(owner)->getCommands();
    VkBufferMemoryBarrier barrier = init::BufferMemoryBarrier();
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstAccessMask = dstAccess;
    barrier.srcAccessMask = srcAccess;
    barrier.offset = 0;
    barrier.size = size;
    VkBufferMemoryBarrier dynamicBarriers[Gfx::numFramesBuffered];
    for(uint32 i = 0; i < numBuffers; ++i)
    {
        dynamicBarriers[i] = barrier;
        dynamicBarriers[i].buffer = buffers[i].buffer;
    }
    vkCmdPipelineBarrier(commandBuffer->getHandle(), srcStage, dstStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
}

void * Buffer::lock(bool writeOnly)
{
    return lockRegion(0, size, writeOnly);
}

void * Buffer::lockRegion(uint64 regionOffset, uint64 regionSize, bool writeOnly)
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
        OStagingBuffer stagingBuffer = graphics->getStagingManager()->allocateStagingBuffer(regionSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        data = stagingBuffer->getMappedPointer();
        pending.stagingBuffer = std::move(stagingBuffer);
    }
    else
    {
        PCmdBuffer current = graphics->getQueueCommands(owner)->getCommands();
        current->waitForCommand();

        requestOwnershipTransfer(Gfx::QueueType::DEDICATED_TRANSFER);
        VkCommandBuffer handle = graphics->getQueueCommands(owner)->getCommands()->getHandle();

        VkBufferMemoryBarrier barrier =
            init::BufferMemoryBarrier();
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        barrier.buffer = buffers[currentBuffer].buffer;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.offset = 0;
        barrier.size = size;
        vkCmdPipelineBarrier(handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

        OStagingBuffer stagingBuffer = graphics->getStagingManager()->allocateStagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);

        VkBufferCopy regions;
        regions.size = size;
        regions.srcOffset = 0;
        regions.dstOffset = 0;

        vkCmdCopyBuffer(handle, buffers[currentBuffer].buffer, stagingBuffer->getHandle(), 1, &regions);

        graphics->getQueueCommands(owner)->submitCommands();
        vkQueueWaitIdle(graphics->getQueueCommands(owner)->getQueue()->getHandle());
        stagingBuffer->getMappedPointer(); // this maps the memory if not mapped already
        stagingBuffer->flushMappedMemory();
        data = stagingBuffer->getMappedPointer();

        pending.stagingBuffer = std::move(stagingBuffer);

    }
    pendingBuffers[this] = std::move(pending);

    assert(data);
    return data;
}

void Buffer::unlock()
{
    auto found = pendingBuffers.find(this);
    if (found != pendingBuffers.end())
    {
        PendingBuffer& pending = found->second;
        pending.stagingBuffer->flushMappedMemory();
        if (pending.writeOnly)
        {
            PStagingBuffer stagingBuffer = pending.stagingBuffer;
            PCmdBuffer cmdBuffer = graphics->getQueueCommands(owner)->getCommands();
            VkCommandBuffer cmdHandle = cmdBuffer->getHandle();

            VkBufferCopy region;
            std::memset(&region, 0, sizeof(VkBufferCopy));
            region.size = pending.size;
            region.dstOffset = pending.offset;
            vkCmdCopyBuffer(cmdHandle, stagingBuffer->getHandle(), buffers[currentBuffer].buffer, 1, &region);
            graphics->getQueueCommands(owner)->submitCommands();
        }
        //requestOwnershipTransfer(pending.prevQueue);
        graphics->getStagingManager()->releaseStagingBuffer(std::move(pending.stagingBuffer));
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
        dedicatedStagingBuffer = graphics->getStagingManager()->allocateStagingBuffer(createInfo.sourceData.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    }
    if (createInfo.sourceData.data != nullptr)
    {
        void *data = lock();
        std::memcpy(data, createInfo.sourceData.data, createInfo.sourceData.size);
        unlock();
    }
}

UniformBuffer::~UniformBuffer()
{
    graphics->getStagingManager()->releaseStagingBuffer(std::move(dedicatedStagingBuffer));
}

bool UniformBuffer::updateContents(const DataSource &sourceData) 
{
    if(!Gfx::UniformBuffer::updateContents(sourceData))
    {
        // no update was performed, skip
        return false;
    }
    void* data = lock();
    std::memcpy(data, sourceData.data, sourceData.size);
    unlock();
    return true;
}
void* UniformBuffer::lock(bool writeOnly)
{
    if(dedicatedStagingBuffer != nullptr)
    {
        return dedicatedStagingBuffer->getMappedPointer();
    }
    return Vulkan::Buffer::lock(writeOnly);
}

void UniformBuffer::unlock()
{
    if(dedicatedStagingBuffer != nullptr)
    {
        dedicatedStagingBuffer->flushMappedMemory();
        PCmdBuffer cmdBuffer = graphics->getQueueCommands(currentOwner)->getCommands();
        VkCommandBuffer cmdHandle = cmdBuffer->getHandle();

        VkBufferCopy region;
        std::memset(&region, 0, sizeof(VkBufferCopy));
        region.size = Vulkan::Buffer::size;
        vkCmdCopyBuffer(cmdHandle, dedicatedStagingBuffer->getHandle(), buffers[currentBuffer].buffer, 1, &region);
        graphics->getQueueCommands(currentOwner)->submitCommands();
    }
    else
    {
        Vulkan::Buffer::unlock();
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
    : Gfx::ShaderBuffer(graphics->getFamilyMapping(), sourceData.stride, sourceData.sourceData.size / sourceData.stride, sourceData.sourceData)
    , Vulkan::Buffer(graphics, sourceData.sourceData.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, currentOwner, sourceData.dynamic)
    , dedicatedStagingBuffer(nullptr)
{
    if(sourceData.dynamic)
    {
        dedicatedStagingBuffer = graphics->getStagingManager()->allocateStagingBuffer(sourceData.sourceData.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    }
    if (sourceData.sourceData.data != nullptr)
    {
        void *data = lock();
        std::memcpy(data, sourceData.sourceData.data, sourceData.sourceData.size);
        unlock();
    }
}

ShaderBuffer::~ShaderBuffer()
{
    graphics->getStagingManager()->releaseStagingBuffer(std::move(dedicatedStagingBuffer));
}

bool ShaderBuffer::updateContents(const DataSource &sourceData) 
{
    assert(sourceData.size <= getSize());
    Gfx::ShaderBuffer::updateContents(sourceData);
    //We always want to update, as the contents could be different on the GPU
    void* data = lock();
    std::memcpy(data, sourceData.data, sourceData.size);
    unlock();
    return true;
}
void* ShaderBuffer::lock(bool writeOnly)
{
    if(dedicatedStagingBuffer != nullptr)
    {
        return dedicatedStagingBuffer->getMappedPointer();
    }
    return Vulkan::Buffer::lock(writeOnly);
}

void ShaderBuffer::unlock()
{
    if(dedicatedStagingBuffer != nullptr)
    {
        dedicatedStagingBuffer->flushMappedMemory();
        PCmdBuffer cmdBuffer = graphics->getQueueCommands(currentOwner)->getCommands();
        VkCommandBuffer cmdHandle = cmdBuffer->getHandle();

        VkBufferCopy region;
        std::memset(&region, 0, sizeof(VkBufferCopy));
        region.size = Vulkan::Buffer::size;
        vkCmdCopyBuffer(cmdHandle, dedicatedStagingBuffer->getHandle(), buffers[currentBuffer].buffer, 1, &region);
        graphics->getQueueCommands(currentOwner)->submitCommands();
    }
    else
    {
        Vulkan::Buffer::unlock();
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
        void *data = lock();
        std::memcpy(data, sourceData.sourceData.data, sourceData.sourceData.size);
        unlock();
    }
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::updateRegion(DataSource update)
{
    void* data = lockRegion(update.offset, update.size);
    std::memcpy(data, update.data, update.size);
    unlock();
}

void VertexBuffer::download(Array<uint8>& buffer)
{
    void* data = lock(false);
    buffer.resize(size);
    std::memcpy(buffer.data(), data, size);
    unlock();
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
        void *data = lock();
        std::memcpy(data, sourceData.sourceData.data, sourceData.sourceData.size);
        unlock();
    }
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::download(Array<uint8>& buffer)
{
    void* data = lock(false);
    buffer.resize(size);
    std::memcpy(buffer.data(), data, size);
    unlock();
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