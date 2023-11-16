#include "Allocator.h"
#include "Graphics.h"
#include "Resources.h"
#include "Enums.h"
#include "Command.h"

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

void *SubAllocation::map()
{
    return (uint8 *)owner->map() + alignedOffset;
}

void SubAllocation::flushMemory()
{
    owner->flushMemory();
}

void SubAllocation::invalidate()
{
    owner->invalidate();
}

Allocation::Allocation(PGraphics graphics, PAllocator pool, VkDeviceSize size, uint8 memoryTypeIndex,
                       VkMemoryPropertyFlags properties, VkMemoryDedicatedAllocateInfo *dedicatedInfo)
    : device(graphics->getDevice())
    , pool(pool)
    , bytesAllocated(size)
    , bytesUsed(0)
    , mappedPointer(nullptr)
    , canMap((properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    , isMapped(false)
    , properties(properties)
    , memoryTypeIndex(memoryTypeIndex)
{
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = dedicatedInfo,
        .allocationSize = size,
        .memoryTypeIndex = memoryTypeIndex,
    };
    isDedicated = dedicatedInfo != nullptr;
    VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &allocatedMemory));
    freeRanges[0] = size;
}

Allocation::~Allocation()
{
    vkFreeMemory(device, allocatedMemory, nullptr);
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
        if (size >= allocatedSize)
        {
            //std::cout << "Allocating " << lower << "-" << lower + allocatedSize << std::endl;
            VkDeviceSize newSize = size - allocatedSize;
            VkDeviceSize newLower = lower + allocatedSize;
            OSubAllocation alloc = new SubAllocation(this, requestedSize, lower, allocatedSize, alignedOffset);
            activeAllocations.add(alloc);
            freeRanges.erase(lower);
            if (newSize > 0)
            {
                freeRanges[newLower] = newSize;
            }
            bytesUsed += allocatedSize;
            return alloc;
        }
    }
    return nullptr;
}

void Allocation::markFree(PSubAllocation allocation)
{
    //std::cout << "Freeing " << allocation->allocatedOffset << "-" << allocation->allocatedOffset + allocation->allocatedSize << std::endl;
    assert(activeAllocations.find(allocation) != activeAllocations.end());
    VkDeviceSize lowerBound = allocation->allocatedOffset;
    VkDeviceSize upperBound = allocation->allocatedOffset + allocation->allocatedSize;
    freeRanges[lowerBound] = allocation->allocatedSize;
    for (const auto& [lower, size] : freeRanges)
    {
        if (lower + size == lowerBound)
        {
            freeRanges[lower] = size + allocation->allocatedSize;
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

void Allocation::invalidate()
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
        //std::cout << "Creating heap " << i << " with properties " << memoryHeap.flags << " size " << memoryHeap.size << std::endl;
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
            std::cout << "Heap " << heapIndex << ": " << (float)heaps[heapIndex].inUse / heaps[heapIndex].maxSize * 100 << "%" << std::endl; 
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
    OAllocation newAllocation = new Allocation(graphics, this, (requirements.size > DEFAULT_ALLOCATION) ? requirements.size : DEFAULT_ALLOCATION, memoryTypeIndex, properties, nullptr);
    heaps[heapIndex].inUse += newAllocation->bytesAllocated;
    std::cout << "Heap " << heapIndex << ": " << (float)heaps[heapIndex].inUse / heaps[heapIndex].maxSize * 100 << "%" << std::endl;
    heaps[heapIndex].allocations.add(std::move(newAllocation));
    return heaps[heapIndex].allocations.back()->getSuballocation(requirements.size, requirements.alignment);
}

void Allocator::free(PAllocation allocation)
{
    //std::cout << "Freeing allocation" << std::endl;
    for (uint32 heapIndex = 0; heapIndex < heaps.size(); ++heapIndex)
    {
        for (uint32 alloc = 0; alloc < heaps[heapIndex].allocations.size(); ++alloc)
        {
            if (heaps[heapIndex].allocations[alloc] == allocation)
            {
                heaps[heapIndex].inUse -= allocation->bytesAllocated;
                std::cout << "Heap " << heapIndex << ": " << (float)heaps[heapIndex].inUse / heaps[heapIndex].maxSize * 100 << "%" << std::endl;
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

StagingBuffer::StagingBuffer(PGraphics graphics, OSubAllocation allocation, VkBuffer buffer, VkDeviceSize size)
    : QueueOwnedResource(graphics->getFamilyMapping(), Gfx::QueueType::DEDICATED_TRANSFER)
    , graphics(graphics)
    , allocation(std::move(allocation))
    , buffer(buffer)
    , size(size)
{
}

StagingBuffer::~StagingBuffer()
{
    graphics->getDestructionManager()->queueBuffer(
        graphics->getDedicatedTransferCommands()->getCommands(), buffer);
    graphics->getDestructionManager()->queueAllocation(
        graphics->getDedicatedTransferCommands()->getCommands(), std::move(allocation));
}

void* StagingBuffer::map()
{
    return allocation->map();
}

void StagingBuffer::flush()
{
    allocation->flushMemory();
}

void StagingBuffer::invalidate()
{
    allocation->invalidate();
}

VkDeviceMemory StagingBuffer::getMemory() const
{
    return allocation->getHandle();
}

VkDeviceSize StagingBuffer::getOffset() const
{
    return allocation->getOffset();
}

void StagingBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    assert(false);
}
void StagingBuffer::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
    Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage)
{
    assert(false);
}

StagingManager::StagingManager(PGraphics graphics, PAllocator pool)
    : graphics(graphics), pool(pool)
{
}

StagingManager::~StagingManager()
{
}

OStagingBuffer StagingManager::create(uint64 size)
{
    //std::cout << "Creating new stagingbuffer" << std::endl;
    uint32 queueIndex = graphics->getFamilyMapping().getQueueTypeFamilyIndex(Gfx::QueueType::DEDICATED_TRANSFER);
    VkBufferCreateInfo stagingBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &queueIndex,
    };
    
    VkBuffer buffer;
    VK_CHECK(vkCreateBuffer(graphics->getDevice(), &stagingBufferCreateInfo, nullptr, &buffer));

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
    vkGetBufferMemoryRequirements2(graphics->getDevice(), &bufferQuery, &memReqs);

    OStagingBuffer stagingBuffer = new StagingBuffer(
        graphics,
        pool->allocate(memReqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT, buffer),
        buffer,
        size
    );
    vkBindBufferMemory(graphics->getDevice(), buffer, stagingBuffer->getMemory(), stagingBuffer->getOffset());
    
    return stagingBuffer;
}
