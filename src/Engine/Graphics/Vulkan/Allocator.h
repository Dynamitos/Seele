#pragma once
#include "MinimalEngine.h"
#include "Enums.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include <mutex>

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Graphics)
DECLARE_REF(Allocator)
DECLARE_REF(Allocation)
class SubAllocation
{
public:
	SubAllocation(PAllocation owner, VkDeviceSize requestedSize, VkDeviceSize allocatedOffset, VkDeviceSize allocatedSize, VkDeviceSize alignedOffset);
	~SubAllocation();
	VkDeviceMemory getHandle() const;

	constexpr VkDeviceSize getSize() const
	{
		return requestedSize;
	}

	constexpr VkDeviceSize getOffset() const
	{
		return alignedOffset;
	}

	bool isReadable() const;

	void *getMappedPointer();
	void flushMemory();
	void invalidateMemory();

private:
	PAllocation owner;
	VkDeviceSize requestedSize;
	VkDeviceSize allocatedOffset;
	VkDeviceSize allocatedSize;
	VkDeviceSize alignedOffset;
	friend class Allocation;
	friend class Allocator;
};
DEFINE_REF(SubAllocation)
class Allocation
{
public:
	Allocation(PGraphics graphics, PAllocator allocator, VkDeviceSize size, uint8 memoryTypeIndex,
			   VkMemoryPropertyFlags properties, VkMemoryDedicatedAllocateInfo *dedicatedInfo = nullptr);
	~Allocation();
	OSubAllocation getSuballocation(VkDeviceSize size, VkDeviceSize alignment);
	void markFree(PSubAllocation alloc);
	constexpr VkDeviceMemory getHandle() const
	{
		return allocatedMemory;
	}
	
	constexpr void* getMappedPointer()
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

	constexpr bool isReadable() const
	{
		return readable;
	}

	void flushMemory();
	void invalidateMemory();

private:
	VkDevice device;
	PAllocator allocator;
	VkDeviceSize bytesAllocated;
	VkDeviceSize bytesUsed;
	VkDeviceMemory allocatedMemory;
	Array<PSubAllocation> activeAllocations;
	Map<VkDeviceSize, VkDeviceSize> freeRanges;
	void *mappedPointer;
	uint8 isDedicated : 1;
	uint8 canMap : 1;
	uint8 isMapped : 1;
	uint8 readable : 1;
	VkMemoryPropertyFlags properties;
	uint8 memoryTypeIndex;
	friend class Allocator;
};
DEFINE_REF(Allocation)

class Allocator
{
public:
	Allocator(PGraphics graphics);
	~Allocator();
	OSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
							VkMemoryDedicatedAllocateInfo *dedicatedInfo = nullptr);
	OSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
								   VkBuffer buffer)
	{
		VkMemoryDedicatedAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.buffer = buffer;
		allocInfo.image = VK_NULL_HANDLE;
		return allocate(requirements, props, &allocInfo);
	}
	OSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
								   VkImage image)
	{
		VkMemoryDedicatedAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.buffer = VK_NULL_HANDLE;
		allocInfo.image = image;
		return allocate(requirements, props, &allocInfo);
	}

	void free(PAllocation allocation);
private:
	enum
	{
		MemoryBlockSize = 16 * 1024 * 1024 // 16MB
	};
	struct HeapInfo
	{
		VkDeviceSize maxSize = 0;
		VkDeviceSize inUse = 0;
		Array<OAllocation> allocations;
		HeapInfo() {}
		HeapInfo(const HeapInfo& other) = delete;
		HeapInfo(HeapInfo&& other) = default;
		HeapInfo& operator=(const HeapInfo& other) = delete;
		HeapInfo& operator=(HeapInfo&& other) = default;
	};
	Array<HeapInfo> heaps;
	uint32 findMemoryType(uint32 typeBits, VkMemoryPropertyFlags properties);
	PGraphics graphics;
	VkPhysicalDeviceMemoryProperties memProperties;
};
DEFINE_REF(Allocator)
DECLARE_REF(StagingManager)
class StagingBuffer
{
public:
	StagingBuffer(OSubAllocation allocation, VkBuffer buffer, VkDeviceSize size, VkBufferUsageFlags usage, uint8 readable);
	~StagingBuffer();
	void* getMappedPointer();
	void flushMappedMemory();
	void invalidateMemory();
	constexpr VkBuffer getHandle() const
	{
		return buffer;
	}
	VkDeviceMemory getMemoryHandle() const;
	VkDeviceSize getOffset() const;
	uint64 getSize() const;
	constexpr bool isReadable() const
	{
		return readable;
	}

	constexpr VkBufferUsageFlags getUsage() const
	{
		return usage;
	}

private:
	OSubAllocation allocation;
	VkBuffer buffer;
	VkDeviceSize size;
	VkBufferUsageFlags usage;
	uint8 readable;
};
DEFINE_REF(StagingBuffer)

class StagingManager
{
public:
	StagingManager(PGraphics graphics, PAllocator allocator);
	~StagingManager();
	OStagingBuffer allocateStagingBuffer(uint64 size, VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bool bCPURead = false);
	void releaseStagingBuffer(OStagingBuffer buffer);
	void clearPending();

private:
	PGraphics graphics;
	PAllocator allocator;
	Array<OStagingBuffer> freeBuffers;
	Array<PStagingBuffer> activeBuffers;
};
DEFINE_REF(StagingManager)
} // namespace Vulkan
} // namespace Seele