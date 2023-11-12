#include "Allocator.h"
#include "Graphics.h"
#include "Initializer.h"

using namespace Seele::Vulkan;

SubAllocation::SubAllocation(PAllocation owner, VkDeviceSize requestedSize, VkDeviceSize allocatedOffset, VkDeviceSize allocatedSize, VkDeviceSize alignedOffset)
    : owner(owner)
    , requestedSize(requestedSize)
    , allocatedOffset(allocatedOffset)
    , allocatedSize(allocatedSize)
    , alignedOffset(alignedOffset)
{
}

SubAllocation::~SubAllocation()
{
    owner->markFree(this);
}

VkDeviceMemory SubAllocation::getHandle() const
{
    return owner->getHandle();
}

bool SubAllocation::isReadable() const
{
    return owner->isReadable();
}

void *SubAllocation::getMappedPointer()
{
    return (uint8 *)owner->getMappedPointer() + alignedOffset;
}

void SubAllocation::flushMemory()
{
    owner->flushMemory();
}

void SubAllocation::invalidateMemory()
{
    owner->invalidateMemory();
}

Allocation::Allocation(PGraphics graphics, PAllocator allocator, VkDeviceSize size, uint8 memoryTypeIndex,
                       VkMemoryPropertyFlags properties, VkMemoryDedicatedAllocateInfo *dedicatedInfo)
    : device(graphics->getDevice())
    , allocator(allocator)
    , bytesAllocated(0)
    , bytesUsed(0)
    , readable(properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    , properties(properties)
    , memoryTypeIndex(memoryTypeIndex)
{
    VkMemoryAllocateInfo allocInfo =
        init::MemoryAllocateInfo();
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    isDedicated = dedicatedInfo != nullptr;
    allocInfo.pNext = dedicatedInfo;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &allocatedMemory));
    bytesAllocated = size;
    freeRanges[0] = size;

    canMap = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    isMapped = false;
}

Allocation::~Allocation()
{
    assert(bytesUsed == 0);
}

OSubAllocation Allocation::getSuballocation(VkDeviceSize requestedSize, VkDeviceSize alignment)
{
    if (isDedicated)
    {
        if (activeAllocations.empty() && requestedSize == bytesAllocated)
        {
            OSubAllocation suballoc = new SubAllocation(this, requestedSize, 0, requestedSize, 0);
            activeAllocations.add(suballoc);
            freeRanges.clear();
            bytesUsed += requestedSize;
            return suballoc;
        }
        else
        {
            return nullptr;
        }
    }
    for (const auto& [lower, size] : freeRanges)
    {
        VkDeviceSize alignedOffset = lower + alignment - 1;
        alignedOffset /= alignment;
        alignedOffset *= alignment;
        VkDeviceSize allocatedSize = requestedSize + (alignedOffset - lower);
        if (size == allocatedSize)
        {
            OSubAllocation alloc = new SubAllocation(this, requestedSize, lower, allocatedSize, alignedOffset);
            activeAllocations.add(alloc);
            freeRanges.erase(lower);
            bytesUsed += allocatedSize;
            return alloc;
        }
        else if (allocatedSize < size)
        {
            VkDeviceSize newSize = size - allocatedSize;
            VkDeviceSize newLower = lower + allocatedSize;
            OSubAllocation subAlloc = new SubAllocation(this, requestedSize, lower, allocatedSize, alignedOffset);
            activeAllocations.add(subAlloc);
            freeRanges.erase(lower);
            freeRanges[newLower] = newSize;
            bytesUsed += allocatedSize;
            return subAlloc;
        }
    }
    return nullptr;
}

void Allocation::markFree(PSubAllocation allocation)
{
    assert(activeAllocations.find(allocation) != activeAllocations.end());
    VkDeviceSize lowerBound = allocation->allocatedOffset;
    VkDeviceSize upperBound = allocation->allocatedOffset + allocation->allocatedSize;
    freeRanges[lowerBound] = allocation->allocatedSize;
    for (const auto& [lower, size] : freeRanges)
    {
        if (lower + size == lowerBound)
        {
            freeRanges[lower] = upperBound;
            freeRanges.erase(lowerBound);
            lowerBound = lower;
            break;
        }
    }
    if (freeRanges.find(upperBound) != freeRanges.end())
    {
        freeRanges[lowerBound] += freeRanges[upperBound];
        freeRanges.erase(upperBound);
    }
    activeAllocations.remove(allocation, false);
    bytesUsed -= allocation->allocatedSize;
    if (bytesUsed == 0)
    {
        allocator->free(this);
    }
}

void Allocation::flushMemory()
{
    VkMappedMemoryRange range = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = 0,
        .memory = allocatedMemory,
        .offset = 0,
        .size = bytesAllocated,
    };
    vkFlushMappedMemoryRanges(device, 1, &range);
}

void Allocation::invalidateMemory()
{
    VkMappedMemoryRange range = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = 0,
        .memory = allocatedMemory,
        .size = bytesAllocated,
    };
    vkInvalidateMappedMemoryRanges(device, 1, &range);
}

Allocator::Allocator(PGraphics graphics)
    : graphics(graphics)
{
    vkGetPhysicalDeviceMemoryProperties(graphics->getPhysicalDevice(), &memProperties);
    heaps.reserve(memProperties.memoryHeapCount);
    for (size_t i = 0; i < memProperties.memoryHeapCount; ++i)
    {
        VkMemoryHeap memoryHeap = memProperties.memoryHeaps[i];
        HeapInfo heapInfo; 
        heapInfo.maxSize = memoryHeap.size;
        std::cout << "Creating heap " << i << " with properties " << memoryHeap.flags << " size " << memoryHeap.size << std::endl;
        heaps.add(std::move(heapInfo));
    }
}

Allocator::~Allocator()
{
    for (auto& heap : heaps)
    {
        for (auto& alloc : heap.allocations)
        {
            assert(alloc->activeAllocations.empty());
            assert(alloc->freeRanges.size() == 1);
        }
        heap.allocations.clear();
    }
    graphics = nullptr;
}

OSubAllocation Allocator::allocate(const VkMemoryRequirements2 &memRequirements2, VkMemoryPropertyFlags properties, VkMemoryDedicatedAllocateInfo *dedicatedInfo)
{
    const VkMemoryRequirements &requirements = memRequirements2.memoryRequirements;
    uint32 memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, properties);
    uint32 heapIndex = memProperties.memoryTypes[memoryTypeIndex].heapIndex;

    if (memRequirements2.pNext != nullptr)
    {
        VkMemoryDedicatedRequirements *dedicatedReq = (VkMemoryDedicatedRequirements *)memRequirements2.pNext;
        if (dedicatedReq->prefersDedicatedAllocation)
        {
            OAllocation newAllocation = new Allocation(graphics, this, requirements.size, memoryTypeIndex, properties, dedicatedInfo);
            heaps[heapIndex].inUse += newAllocation->bytesAllocated;
            std::cout << "Heap " << heapIndex << ": " << (float)heaps[heapIndex].inUse / heaps[heapIndex].maxSize << "%" << std::endl; 
            heaps[heapIndex].allocations.add(std::move(newAllocation));
            return heaps[heapIndex].allocations.back()->getSuballocation(requirements.size, requirements.alignment);
        } 
    }
    for (auto& alloc : heaps[heapIndex].allocations)
    {
        if(alloc->memoryTypeIndex == memoryTypeIndex)
        {
            OSubAllocation suballoc = alloc->getSuballocation(requirements.size, requirements.alignment);
            if (suballoc != nullptr)
            {
                return std::move(suballoc);
            }
        }
    }

    // no suitable allocations found, allocate new block
    OAllocation newAllocation = new Allocation(graphics, this, (requirements.size > MemoryBlockSize) ? requirements.size : (VkDeviceSize)MemoryBlockSize, memoryTypeIndex, properties, nullptr);
    heaps[heapIndex].inUse += newAllocation->bytesAllocated;
    std::cout << "Heap " << heapIndex << ": " << (float)heaps[heapIndex].inUse / heaps[heapIndex].maxSize << "%" << std::endl;     heaps[heapIndex].allocations.add(std::move(newAllocation));
    return heaps[heapIndex].allocations.back()->getSuballocation(requirements.size, requirements.alignment);
}

void Allocator::free(PAllocation allocation)
{
    for (uint32 heapIndex = 0; heapIndex < heaps.size(); ++heapIndex)
    {
        for (uint32 alloc = 0; alloc < heaps[heapIndex].allocations.size(); ++alloc)
        {
            if (heaps[heapIndex].allocations[alloc] == allocation)
            {
                heaps[heapIndex].inUse -= allocation->bytesAllocated;
                std::cout << "Heap " << heapIndex << ": " << (float)heaps[heapIndex].inUse / heaps[heapIndex].maxSize << "%" << std::endl;
                heaps[heapIndex].allocations.removeAt(alloc, false);
                return;
            }
        }
    }
}
uint32 Allocator::findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    for (uint32 i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    throw std::runtime_error("error finding memory");
}

StagingBuffer::StagingBuffer(OSubAllocation allocation, VkBuffer buffer, VkDeviceSize size, VkBufferUsageFlags usage, uint8 readable)
    : allocation(std::move(allocation))
    , buffer(buffer)
    , size(size)
    , usage(usage)
    , readable(readable)
{
}

StagingBuffer::~StagingBuffer()
{
    assert(allocation == nullptr);
    // buffer went out of scope without being cleaned up
}

void* StagingBuffer::getMappedPointer()
{
    return allocation->getMappedPointer();
}

void StagingBuffer::flushMappedMemory()
{
    allocation->flushMemory();
}

void StagingBuffer::invalidateMemory()
{
    allocation->invalidateMemory();
}

VkDeviceMemory StagingBuffer::getMemoryHandle() const
{
    return allocation->getHandle();
}

VkDeviceSize StagingBuffer::getOffset() const
{
    return allocation->getOffset();
}

uint64 StagingBuffer::getSize() const
{
    return size;
}

StagingManager::StagingManager(PGraphics graphics, PAllocator allocator)
    : graphics(graphics), allocator(allocator)
{
}

StagingManager::~StagingManager()
{
}

void StagingManager::clearPending()
{
}

OStagingBuffer StagingManager::allocateStagingBuffer(uint64 size, VkBufferUsageFlags usage, bool readable)
{
    for (auto it = freeBuffers.begin(); it != freeBuffers.end(); ++it)
    {
        auto& freeBuffer = *it;
        if (freeBuffer->getSize() == size && freeBuffer->isReadable() == readable && freeBuffer->getUsage()  == usage)
        {
            //std::cout << "Reusing staging buffer" << std::endl;
            activeBuffers.add(freeBuffer);
            OStagingBuffer owner = std::move(freeBuffer);
            freeBuffers.remove(it, false);
            return std::move(owner);
        }
    }
    //std::cout << "Creating new stagingbuffer" << std::endl;
    VkBuffer buffer;
    VkBufferCreateInfo stagingBufferCreateInfo = init::BufferCreateInfo(usage, size);
    stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32 queueIndex = graphics->getFamilyMapping().getQueueTypeFamilyIndex(Gfx::QueueType::DEDICATED_TRANSFER);
    stagingBufferCreateInfo.queueFamilyIndexCount = 1;
    stagingBufferCreateInfo.pQueueFamilyIndices = &queueIndex;
    VkDevice vulkanDevice = graphics->getDevice();

    VK_CHECK(vkCreateBuffer(vulkanDevice, &stagingBufferCreateInfo, nullptr, &buffer));

    VkMemoryDedicatedRequirements dedicatedReqs = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS,
        .pNext = nullptr,
    };
    VkMemoryRequirements2 memReqs = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
        .pNext = &dedicatedReqs,
    };
    VkBufferMemoryRequirementsInfo2 bufferQuery = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
        .pNext = nullptr,
        .buffer = buffer,
    };
    vkGetBufferMemoryRequirements2(vulkanDevice, &bufferQuery, &memReqs);

    memReqs.memoryRequirements.alignment =
        (16 > memReqs.memoryRequirements.alignment) ? 16 : memReqs.memoryRequirements.alignment;

    OStagingBuffer stagingBuffer = new StagingBuffer(
        allocator->allocate(memReqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | (readable ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_HOST_CACHED_BIT), buffer),
        buffer,
        size,
        usage,
        readable
    );
    vkBindBufferMemory(graphics->getDevice(), buffer, stagingBuffer->getMemoryHandle(), stagingBuffer->getOffset());

    activeBuffers.add(stagingBuffer);
    
    return std::move(stagingBuffer);
}

void StagingManager::releaseStagingBuffer(OStagingBuffer buffer)
{
    if (buffer == nullptr)
    {
        return;
    }
    activeBuffers.remove(buffer);
    freeBuffers.add(std::move(buffer));
}