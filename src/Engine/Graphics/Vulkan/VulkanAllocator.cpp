#include "VulkanAllocator.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"

using namespace Seele::Vulkan;

SubAllocation::SubAllocation(Allocation *owner, VkDeviceSize allocatedOffset, VkDeviceSize size, VkDeviceSize alignedOffset, VkDeviceSize allocatedSize)
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

Allocation::Allocation(PGraphics graphics, Allocator *allocator, VkDeviceSize size, uint8 memoryTypeIndex,
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
	PSubAllocation freeRange = new SubAllocation(this, 0, size, 0, size);
	freeRanges[0] = freeRange;

	canMap = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	isMapped = false;
}

Allocation::~Allocation()
{
}

PSubAllocation Allocation::getSuballocation(VkDeviceSize requestedSize, VkDeviceSize alignment)
{
	std::unique_lock lck(lock);
	if (isDedicated)
	{
		if (activeAllocations.empty() && requestedSize == bytesAllocated)
		{
			PSubAllocation suballoc = freeRanges[0];
			activeAllocations[0] = suballoc.getHandle();
			freeRanges.clear();
			bytesUsed += requestedSize;
			return suballoc;
		}
		else
		{
			return nullptr;
		}
	}
	for (auto it : freeRanges)
	{
		PSubAllocation freeAllocation = it.value;
		VkDeviceSize allocatedOffset = freeAllocation->allocatedOffset;
		VkDeviceSize alignedOffset = align(allocatedOffset, alignment);
		VkDeviceSize alignmentAdjustment = alignedOffset - allocatedOffset;
		VkDeviceSize size = alignmentAdjustment + requestedSize;
		if (freeAllocation->size == size)
		{
			freeRanges.erase(allocatedOffset);
			activeAllocations[allocatedOffset] = freeAllocation.getHandle();
			bytesUsed += size;
			return freeAllocation;
		}
		else if (size < freeAllocation->allocatedSize)
		{
			freeAllocation->size -= size;
			freeAllocation->allocatedSize -= size;
			freeAllocation->allocatedOffset += size;
			freeAllocation->alignedOffset += size;
			PSubAllocation subAlloc = new SubAllocation(this, allocatedOffset, size, alignedOffset, size);
			activeAllocations[allocatedOffset] = subAlloc.getHandle();
			freeRanges.erase(allocatedOffset);
			freeRanges[freeAllocation->allocatedOffset] = freeAllocation;
			bytesUsed += size;
			return subAlloc;
		}
	}
	return nullptr;
}

void Allocation::markFree(SubAllocation *allocation)
{
	// Dont free if it is already a free allocation, since they also mark themselves on deletion
	if (freeRanges.find(allocation->allocatedOffset) != freeRanges.end())
	{
		return;
	}
	VkDeviceSize lowerBound = allocation->allocatedOffset;
	VkDeviceSize upperBound = allocation->allocatedOffset + allocation->allocatedSize;
	PSubAllocation allocHandle;
	std::unique_lock lck(lock);
	//Join lower bound
	for (auto freeRange : freeRanges)
	{
		PSubAllocation freeAlloc = freeRange.value;
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
			// remove from offset map since key changes
			freeRanges.erase(foundAlloc->key);
			allocHandle->allocatedOffset -= allocation->allocatedSize;
			allocHandle->alignedOffset -= allocation->allocatedSize;
			// place back at correct offset
			freeRanges[allocHandle->allocatedOffset] = allocHandle;
		}
	}
	if (allocHandle == nullptr)
	{
		allocHandle = new SubAllocation(this, allocation->alignedOffset, allocation->size, allocation->alignedOffset, allocation->allocatedSize);
		freeRanges[allocation->allocatedOffset] = allocHandle;
	}
	activeAllocations.erase(allocation->allocatedOffset);
	bytesUsed -= allocation->allocatedSize;

	// TODO: delete allocation when bytesUsed == 0
}

Allocator::Allocator(PGraphics graphics)
	: graphics(graphics)
{
	vkGetPhysicalDeviceMemoryProperties(graphics->getPhysicalDevice(), &memProperties);
	heaps.resize(memProperties.memoryHeapCount);
	for (size_t i = 0; i < memProperties.memoryHeapCount; ++i)
	{
		VkMemoryHeap memoryHeap = memProperties.memoryHeaps[i];
		HeapInfo &heapInfo = heaps[i];
		heapInfo.maxSize = memoryHeap.size;
	}
}

Allocator::~Allocator()
{
	std::unique_lock lck(lock);
	for (auto heap : heaps)
	{
		for (auto alloc : heap.allocations)
		{
			assert(alloc->activeAllocations.empty());
			assert(alloc->freeRanges.size() == 1);
		}
		heap.allocations.clear();
	}
	graphics = nullptr;
}

PSubAllocation Allocator::allocate(const VkMemoryRequirements2 &memRequirements2, VkMemoryPropertyFlags properties, VkMemoryDedicatedAllocateInfo *dedicatedInfo)
{
	std::unique_lock lck(lock);
	const VkMemoryRequirements &requirements = memRequirements2.memoryRequirements;
	uint8 memoryTypeIndex;
	VK_CHECK(findMemoryType(requirements.memoryTypeBits, properties, &memoryTypeIndex));
	uint32 heapIndex = memProperties.memoryTypes[memoryTypeIndex].heapIndex;

	if (memRequirements2.pNext != nullptr)
	{
		VkMemoryDedicatedRequirements *dedicatedReq = (VkMemoryDedicatedRequirements *)memRequirements2.pNext;
		if (dedicatedReq->prefersDedicatedAllocation)
		{
			PAllocation newAllocation = new Allocation(graphics, this, requirements.size, memoryTypeIndex, properties, dedicatedInfo);
			heaps[heapIndex].allocations.add(newAllocation);
			return newAllocation->getSuballocation(requirements.size, requirements.alignment);
		}
	}
	for (auto alloc : heaps[heapIndex].allocations)
	{
		PSubAllocation suballoc = alloc->getSuballocation(requirements.size, requirements.alignment);
		if (suballoc != nullptr)
		{
			return suballoc;
		}
	}

	// no suitable allocations found, allocate new block
	PAllocation newAllocation = new Allocation(graphics, this, (requirements.size > MemoryBlockSize) ? requirements.size : (VkDeviceSize)MemoryBlockSize, memoryTypeIndex, properties, nullptr);
	heaps[heapIndex].allocations.add(newAllocation);
	return newAllocation->getSuballocation(requirements.size, requirements.alignment);
}

void Allocator::free(Allocation *allocation)
{
	std::unique_lock lck(lock);
	for (auto heap : heaps)
	{
		for (uint32 i = 0; i < heap.allocations.size(); ++i)
		{
			if (heap.allocations[i] == allocation)
			{
				heap.allocations.remove(i, false);
				return;
			}
		}
	}
}

VkResult Allocator::findMemoryType(uint32 typeBits, VkMemoryPropertyFlags properties, uint8 *typeIndex)
{
	for (uint8 memoryIndex = 0; memoryIndex < memProperties.memoryTypeCount && typeBits; ++memoryIndex)
	{
		if ((typeBits & 1) == 1)
		{
			if ((memProperties.memoryTypes[memoryIndex].propertyFlags & properties) == properties)
			{
				*typeIndex = memoryIndex;
				return VK_SUCCESS;
			}
		}
		typeBits >>= 1;
	}

	return VK_ERROR_FORMAT_NOT_SUPPORTED;
}

StagingBuffer::StagingBuffer()
{
}

StagingBuffer::~StagingBuffer()
{
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

PStagingBuffer StagingManager::allocateStagingBuffer(uint32 size, VkBufferUsageFlags usage, bool bCPURead)
{
	std::unique_lock l(lock);
	for (auto it = freeBuffers.begin(); it != freeBuffers.end(); ++it)
	{
		auto freeBuffer = *it;
		if (freeBuffer->getSize() == size && freeBuffer->isReadable() == bCPURead && freeBuffer->usage == usage)
		{
			//std::cout << "Reusing staging buffer" << std::endl;
			activeBuffers.add(freeBuffer.getHandle());
			freeBuffers.remove(it, false);
			return freeBuffer;
		}
	}
	//std::cout << "Creating new stagingbuffer" << std::endl;
	PStagingBuffer stagingBuffer = new StagingBuffer();
	VkBufferCreateInfo stagingBufferCreateInfo = init::BufferCreateInfo(usage, size);
	stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkDevice vulkanDevice = graphics->getDevice();

	VK_CHECK(vkCreateBuffer(vulkanDevice, &stagingBufferCreateInfo, nullptr, &stagingBuffer->buffer));

	VkMemoryDedicatedRequirements dedicatedReqs;
	dedicatedReqs.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
	dedicatedReqs.pNext = nullptr;
	VkMemoryRequirements2 memReqs;
	memReqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
	memReqs.pNext = &dedicatedReqs;
	VkBufferMemoryRequirementsInfo2 bufferQuery;
	bufferQuery.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
	bufferQuery.pNext = nullptr;
	bufferQuery.buffer = stagingBuffer->buffer;
	vkGetBufferMemoryRequirements2(vulkanDevice, &bufferQuery, &memReqs);

	memReqs.memoryRequirements.alignment =
		(16 > memReqs.memoryRequirements.alignment) ? 16 : memReqs.memoryRequirements.alignment;
	
	stagingBuffer->allocation = allocator->allocate(memReqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | (bCPURead ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : VK_MEMORY_PROPERTY_HOST_CACHED_BIT), stagingBuffer->buffer);
	stagingBuffer->bReadable = bCPURead;
	stagingBuffer->size = size;
	stagingBuffer->usage = usage;
	vkBindBufferMemory(graphics->getDevice(), stagingBuffer->buffer, stagingBuffer->getMemoryHandle(), stagingBuffer->getOffset());

	activeBuffers.add(stagingBuffer.getHandle());
	
	return stagingBuffer;
}

void StagingManager::releaseStagingBuffer(PStagingBuffer buffer)
{
	std::unique_lock l(lock);
	freeBuffers.add(buffer);
	activeBuffers.remove(activeBuffers.find(buffer.getHandle()));
}