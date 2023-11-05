#include "Allocator.h"
#include "Graphics.h"
#include "Initializer.h"

using namespace Seele::Vulkan;

SubAllocation::SubAllocation(PAllocation owner, VkDeviceSize allocatedOffset, VkDeviceSize size, VkDeviceSize alignedOffset, VkDeviceSize allocatedSize)
    : owner(owner)
    , size(size)
    , allocatedOffset(allocatedOffset)
    , alignedOffset(alignedOffset)
    , allocatedSize(allocatedSize)
{
}

SubAllocation::~SubAllocation()
{
    owner->markFree(this);
}

constexpr VkDeviceMemory SubAllocation::getHandle() const
{
    return owner->getHandle();
}

constexpr VkDeviceSize SubAllocation::getSize() const
{
    return size;
}

constexpr VkDeviceSize SubAllocation::getOffset() const
{
    return alignedOffset;
}

constexpr bool SubAllocation::isReadable() const
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
    freeRanges[0] = new SubAllocation(this, 0, size, 0, size);

    canMap = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    isMapped = false;
}

Allocation::~Allocation()
{
}

OSubAllocation Allocation::getSuballocation(VkDeviceSize requestedSize, VkDeviceSize alignment)
{
    std::scoped_lock lck(lock);
    if (isDedicated)
    {
        if (activeAllocations.empty() && requestedSize == bytesAllocated)
        {
            OSubAllocation suballoc = std::move(freeRanges[0]);
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
    for (auto& it : freeRanges)
    {
        VkDeviceSize allocatedOffset = it.key;
        OSubAllocation& freeAllocation = it.value;
        assert(allocatedOffset == freeAllocation->allocatedOffset);
        VkDeviceSize alignedOffset = allocatedOffset + alignment - 1;
        alignedOffset /= alignment;
        alignedOffset *= alignment;
        VkDeviceSize allocatedSize = requestedSize + (alignedOffset - allocatedOffset);
        if (freeAllocation->size == allocatedSize)
        {
            activeAllocations.add(freeAllocation);
            freeRanges.erase(allocatedOffset);
            bytesUsed += allocatedSize;
            return std::move(freeAllocation);
        }
        else if (allocatedSize < freeAllocation->allocatedSize)
        {
            freeAllocation->size -= allocatedSize;
            freeAllocation->allocatedSize -= allocatedSize;
            freeAllocation->allocatedOffset += allocatedSize;
            freeAllocation->alignedOffset += allocatedSize;
            OSubAllocation subAlloc = new SubAllocation(this, allocatedOffset, allocatedSize, alignedOffset, allocatedSize);
            activeAllocations.add(subAlloc);
            freeRanges[freeAllocation->allocatedOffset] = std::move(freeAllocation);
            freeRanges.erase(allocatedOffset);
            bytesUsed += allocatedSize;
            return subAlloc;
        }
    }
    return nullptr;
}

void Allocation::markFree(PSubAllocation allocation)
{
    // Dont free if it is already a free allocation, since they also mark themselves on deletion
    if (freeRanges.find(allocation->allocatedOffset) != freeRanges.end())
    {
        return;
    }
    VkDeviceSize lowerBound = allocation->allocatedOffset;
    VkDeviceSize upperBound = allocation->allocatedOffset + allocation->allocatedSize;
    PSubAllocation allocHandle;
    PSubAllocation freeRangeToDelete;

    {
        std::scoped_lock lck(lock);
        //Join lower bound
        for (auto& [allocatedOffset, freeAlloc] : freeRanges)
        {
            if (freeAlloc->allocatedOffset <= lowerBound
            && freeAlloc->allocatedOffset + freeAlloc->allocatedSize >= upperBound)
            {
                // allocation is already in a free region
                assert(false);
            }
            if (freeAlloc->allocatedOffset + freeAlloc->allocatedSize == lowerBound)
            {
                //extend freeAlloc by the allocatedSize
                freeAlloc->allocatedSize += allocation->allocatedSize;
                allocHandle = freeAlloc;
                break;
            }
        }
        //Join upper bound
        auto foundAlloc = freeRanges.find(upperBound);
        if (foundAlloc != freeRanges.end())
        {
            // There is a free allocation ending where the new free one ends
            freeRangeToDelete = foundAlloc->value;
            if (allocHandle != nullptr)
            {
                // extend allocHandle by another foundAlloc->allocatedSize bytes
                allocHandle->allocatedSize += foundAlloc->value->allocatedSize;
                freeRanges.erase(foundAlloc->key);
            }
            else
            {
                // set foundAlloc back by size amount
                allocHandle = foundAlloc->value;
                allocHandle->allocatedOffset -= allocation->allocatedSize;
                allocHandle->alignedOffset -= allocation->allocatedSize;
                allocHandle->size += allocation->allocatedSize;
                allocHandle->allocatedSize += allocation->allocatedSize;
                // place back at correct offset, move original owning pointer
                freeRanges[allocHandle->allocatedOffset] = std::move(foundAlloc->value);
                // remove from offset map since key changes
                freeRanges.erase(foundAlloc->key);
            }
        }
        
        if (allocHandle == nullptr)
        {
            freeRanges[allocation->allocatedOffset] = new SubAllocation(this, allocation->allocatedOffset, allocation->size, allocation->alignedOffset, allocation->allocatedSize);
        }
        activeAllocations.remove_if([&](const PSubAllocation& a) {return a.getHandle() == allocation.getHandle(); });
    }
    bytesUsed -= allocation->allocatedSize;
    if(bytesUsed == 0)
    {
        allocator->free(this);
    }
}

constexpr VkDeviceMemory Allocation::getHandle() const
{
    return allocatedMemory;
}

constexpr void* Allocation::getMappedPointer()
{
    if (!canMap)
    {
        return nullptr;
    }
    if (!isMapped)
    {
        vkMapMemory(device, allocatedMemory, 0, bytesAllocated, 0, &mappedPointer);
        isMapped = true;
    }
    return mappedPointer;
}

constexpr bool Allocation::isReadable() const
{
    return readable;
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
        heaps.add(std::move(heapInfo));
    }
}

Allocator::~Allocator()
{
    std::scoped_lock lck(lock);
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
    std::scoped_lock lck(lock);
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
                return suballoc;
            }
        }
    }

    // no suitable allocations found, allocate new block
    OAllocation newAllocation = new Allocation(graphics, this, (requirements.size > MemoryBlockSize) ? requirements.size : (VkDeviceSize)MemoryBlockSize, memoryTypeIndex, properties, nullptr);
    heaps[heapIndex].inUse += newAllocation->bytesAllocated;
    heaps[heapIndex].allocations.add(std::move(newAllocation));
    return heaps[heapIndex].allocations.back()->getSuballocation(requirements.size, requirements.alignment);
}

void Allocator::free(Allocation *allocation)
{
    std::scoped_lock lck(lock);
    for (auto& heap : heaps)
    {
        for (uint32 i = 0; i < heap.allocations.size(); ++i)
        {
            if (heap.allocations[i] == allocation)
            {
                heap.inUse -= allocation->bytesAllocated;
                heap.allocations.removeAt(i, false);
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

StagingBuffer::StagingBuffer(OSubAllocation allocation, VkBuffer buffer, VkBufferUsageFlags usage, uint8 readable)
    : allocation(std::move(allocation))
    , buffer(buffer)
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

constexpr VkDeviceMemory StagingBuffer::getMemoryHandle() const
{
    return allocation->getHandle();
}

constexpr VkDeviceSize StagingBuffer::getOffset() const
{
    return allocation->getOffset();
}

constexpr uint64 StagingBuffer::getSize() const
{
    return allocation->getSize();
}

constexpr bool StagingBuffer::isReadable() const
{
    return readable;
}

constexpr VkBufferUsageFlags StagingBuffer::getUsage() const
{
    return usage;
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
    std::scoped_lock l(lock);
    for (auto it = freeBuffers.begin(); it != freeBuffers.end(); ++it)
    {
        auto& freeBuffer = *it;
        if (freeBuffer->getSize() == size && freeBuffer->isReadable() == readable && freeBuffer->getUsage()  == usage)
        {
            //std::cout << "Reusing staging buffer" << std::endl;
            activeBuffers.add(freeBuffer);
            freeBuffers.remove(it, false);
            return std::move(freeBuffer);
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
        usage,
        readable
    );
    vkBindBufferMemory(graphics->getDevice(), buffer, stagingBuffer->getMemoryHandle(), stagingBuffer->getOffset());

    activeBuffers.add(stagingBuffer);
    
    return stagingBuffer;
}

void StagingManager::releaseStagingBuffer(OStagingBuffer buffer)
{
    std::scoped_lock l(lock);
    activeBuffers.remove(buffer);
    freeBuffers.add(std::move(buffer));
}