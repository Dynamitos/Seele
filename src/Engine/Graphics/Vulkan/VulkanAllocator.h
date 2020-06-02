#pragma once
#include "MinimalEngine.h"
#include "VulkanGraphicsEnums.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include <mutex>

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Graphics);
class Allocation;
class Allocator;
class SubAllocation
{
public:
	SubAllocation(Allocation *owner, VkDeviceSize allocatedOffset, VkDeviceSize size, VkDeviceSize alignedOffset, VkDeviceSize allocatedSize);
	~SubAllocation();
	VkDeviceMemory getHandle() const;
	inline VkDeviceSize getSize() const
	{
		return size;
	}
	inline VkDeviceSize getOffset() const
	{
		return alignedOffset;
	}
	inline bool isReadable() const;
	void *getMappedPointer();
	void flushMemory();
	void invalidateMemory();

private:
	Allocation *owner;
	VkDeviceSize allocatedOffset;
	VkDeviceSize size;
	VkDeviceSize alignedOffset;
	VkDeviceSize allocatedSize;
	friend class Allocation;
	friend class Allocator;
};
DEFINE_REF(SubAllocation);
class Allocation
{
public:
	Allocation(PGraphics graphics, Allocator *allocator, VkDeviceSize size, uint8 memoryTypeIndex,
			   VkMemoryPropertyFlags properties, VkMemoryDedicatedAllocateInfo *dedicatedInfo = nullptr);
	~Allocation();
	PSubAllocation getSuballocation(VkDeviceSize size, VkDeviceSize alignment);
	void markFree(SubAllocation *alloc);
	inline VkDeviceMemory getHandle() const
	{
		return allocatedMemory;
	}
	inline void *getMappedPointer()
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

	inline bool isReadable() const
	{
		return readable;
	}

	void flushMemory()
	{
		VkMappedMemoryRange range;
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.pNext = 0;
		range.memory = allocatedMemory;
		range.size = bytesAllocated;
		range.offset = 0;
		vkFlushMappedMemoryRanges(device, 1, &range);
	}

	void invalidateMemory()
	{
		VkMappedMemoryRange range;
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.pNext = 0;
		range.memory = allocatedMemory;
		range.size = bytesAllocated;
		vkInvalidateMappedMemoryRanges(device, 1, &range);
	}

private:
	Allocator *allocator;
	VkDevice device;
	VkDeviceMemory allocatedMemory;
	VkDeviceSize bytesAllocated;
	VkDeviceSize bytesUsed;
	VkMemoryPropertyFlags properties;
	Map<VkDeviceSize, SubAllocation *> activeAllocations;
	Map<VkDeviceSize, PSubAllocation> freeRanges;
	std::mutex lock;
	void *mappedPointer;
	uint8 memoryTypeIndex;
	uint8 isDedicated : 1;
	uint8 canMap : 1;
	uint8 isMapped : 1;
	uint8 readable : 1;
	friend class Allocator;
};
DEFINE_REF(Allocation);

class Allocator
{
public:
	Allocator(PGraphics graphics);
	~Allocator();
	PSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
							VkMemoryDedicatedAllocateInfo *dedicatedInfo = nullptr);
	inline PSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
								   VkBuffer buffer)
	{
		VkMemoryDedicatedAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.buffer = buffer;
		allocInfo.image = VK_NULL_HANDLE;
		return allocate(requirements, props, &allocInfo);
	}
	inline PSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
								   VkImage image)
	{
		VkMemoryDedicatedAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.buffer = VK_NULL_HANDLE;
		allocInfo.image = image;
		return allocate(requirements, props, &allocInfo);
	}

	void free(Allocation *allocation);

private:
	enum
	{
		MemoryBlockSize = 64 * 1024 * 1024 // 64MB
	};
	struct HeapInfo
	{
		VkDeviceSize maxSize = 0;
		Array<PAllocation> allocations;
	};
	Array<HeapInfo> heaps;
	VkResult findMemoryType(uint32 typeBits, VkMemoryPropertyFlags properties, uint8 *typeIndex);
	std::mutex lock;
	PGraphics graphics;
	VkPhysicalDeviceMemoryProperties memProperties;
};
DEFINE_REF(Allocator);

class StagingBuffer
{
public:
	StagingBuffer();
	~StagingBuffer();
	void *getMappedPointer()
	{
		return allocation->getMappedPointer();
	}
	void flushMappedMemory()
	{
		allocation->flushMemory();
	}
	void invalidateMemory()
	{
		allocation->invalidateMemory();
	}
	VkBuffer getHandle() const
	{
		return buffer;
	}
	VkDeviceMemory getMemoryHandle() const
	{
		return allocation->getHandle();
	}
	VkDeviceSize getOffset() const
	{
		return allocation->getOffset();
	}

private:
	PSubAllocation allocation;
	VkBuffer buffer;
	uint8 bReadable;
	friend class StagingManager;
};
DEFINE_REF(StagingBuffer);

class StagingManager
{
public:
	StagingManager(PGraphics graphics, PAllocator allocator);
	~StagingManager();
	PStagingBuffer allocateStagingBuffer(uint32 size, VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bool bCPURead = false);
	void releaseStagingBuffer(PStagingBuffer buffer);
	void clearPending();

private:
	PGraphics graphics;
	PAllocator allocator;
	Array<PStagingBuffer> freeBuffers;
	Array<StagingBuffer *> activeBuffers;
};
DEFINE_REF(StagingManager);
} // namespace Vulkan
} // namespace Seele