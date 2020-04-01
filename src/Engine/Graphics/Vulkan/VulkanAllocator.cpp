#include "VulkanAllocator.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"
#include "Math/MemCRC.h"

using namespace Seele::Vulkan;

SubAllocation::SubAllocation(Allocation* owner, uint32 allocatedOffset, uint32 size, uint32 alignedOffset, uint32 allocatedSize)
	: owner(owner)
	, size(size)
	, allocatedOffset(allocatedOffset)
	, alignedOffset(alignedOffset)
	, allocatedSize(allocatedSize)
{
}

Allocation::Allocation(PGraphics graphics, Allocator* allocator, uint32 size, uint32 memoryTypeIndex, VkMemoryPropertyFlags properties, bool isDedicated)
	: device(graphics->getDevice())
	, allocator(allocator)
	, bytesAllocated(0)
	, bytesUsed(0)
	, properties(properties)
	, memoryTypeIndex(memoryTypeIndex)
{
	VkMemoryAllocateInfo allocInfo =
		init::MemoryAllocateInfo();
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = memoryTypeIndex;
	VK_CHECK(vkAllocateMemory(device, &allocInfo, nullptr, &allocatedMemory));
	bytesAllocated = size;
	PSubAllocation freeRange = new SubAllocation(this, 0, size, 0, size);
	freeRanges.add(freeRange);
}

PSubAllocation Allocation::getSuballocation(uint32 requestedSize, uint32 alignment)
{
	if (isDedicated)
	{
		if (activeAllocations.size() == 0)
		{
			assert(requestedSize == bytesAllocated);
			activeAllocations.add(freeRanges.back());
			freeRanges.clear();
		}
		else
		{
			return nullptr;
		}
	}
	for (int i = 0; i < freeRanges.size(); ++i)
	{
		PSubAllocation freeAllocation = freeRanges[i];
		uint32 allocatedOffset = freeAllocation->allocatedOffset;
		uint32 alignedOffset = align(allocatedOffset, alignment);
		uint32 alignmentAdjustment = alignedOffset - allocatedOffset;
		uint32 size = alignmentAdjustment + requestedSize;
		if (freeAllocation->size == size)
		{
			freeRanges.remove(i);
			activeAllocations.add(freeAllocation);
			return freeAllocation;
		}
		else if (size < freeAllocation->allocatedSize)
		{
			freeAllocation->size -= size;
			freeAllocation->allocatedSize -= size;
			freeAllocation->allocatedOffset += allocatedOffset;
			freeAllocation->alignedOffset += allocatedOffset;
			PSubAllocation subAlloc = new SubAllocation(this, allocatedOffset, size, alignedOffset, size);
			activeAllocations.add(subAlloc);
			return subAlloc;
		}
	}
	return nullptr;
}

Allocator::Allocator(PGraphics graphics)
	: graphics(graphics)
{
	vkGetPhysicalDeviceMemoryProperties(graphics->getPhysicalDevice(), &memProperties);
	heaps.resize(memProperties.memoryHeapCount);
	for (size_t i = 0; i < memProperties.memoryHeapCount; i++)
	{
		VkMemoryHeap memoryHeap = memProperties.memoryHeaps[i];
		HeapInfo& heapInfo = heaps[i];
		heapInfo.maxSize = memoryHeap.size;
	}
}

Allocator::~Allocator()
{
}

PSubAllocation Allocator::allocate(uint64 size, const VkMemoryRequirements2& memRequirements2, VkMemoryPropertyFlags properties)
{
	// no suitable allocations found, allocate new block
	const VkMemoryRequirements& requirements = memRequirements2.memoryRequirements;
	uint32 memoryTypeIndex;
	VK_CHECK(findMemoryType(requirements.memoryTypeBits, properties, &memoryTypeIndex));
	
	bool isDedicated = memRequirements2.pNext != nullptr;
	PAllocation newAllocation = new Allocation(graphics, this, MemoryBlockSize, memoryTypeIndex, properties, isDedicated);
	uint32 heapIndex = memProperties.memoryTypes[memoryTypeIndex].heapIndex;
	heaps[heapIndex].allocations.add(newAllocation);
	return newAllocation->getSuballocation(size, requirements.alignment);
}

VkResult Allocator::findMemoryType(uint32 typeBits, VkMemoryPropertyFlags properties, uint32* typeIndex)
{
	for (int memoryIndex = 0; memoryIndex < memProperties.memoryTypeCount && typeBits; ++memoryIndex)
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