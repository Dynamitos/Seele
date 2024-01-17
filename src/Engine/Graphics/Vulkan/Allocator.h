// #pragma once
// #include "MinimalEngine.h"
// #include "Enums.h"
// #include "Containers/Map.h"
// #include "Containers/Array.h"
// #include "Graphics/Resources.h"
// #include <mutex>

// namespace Seele
// {
// namespace Vulkan
// {
// DECLARE_REF(Graphics)
// DECLARE_REF(Allocator)
// DECLARE_REF(Allocation)
// class SubAllocation
// {
// public:
// 	SubAllocation(PAllocation owner, VkDeviceSize requestedSize, VkDeviceSize allocatedOffset, VkDeviceSize allocatedSize, VkDeviceSize alignedOffset);
// 	~SubAllocation();
// 	VkDeviceMemory getHandle() const;

// 	constexpr VkDeviceSize getSize() const
// 	{
// 		return requestedSize;
// 	}

// 	constexpr VkDeviceSize getOffset() const
// 	{
// 		return alignedOffset;
// 	}

// 	void *map();
// 	void flushMemory();
// 	void invalidate();

// private:
// 	PAllocation owner;
// 	VkDeviceSize requestedSize;
// 	VkDeviceSize allocatedOffset;
// 	VkDeviceSize allocatedSize;
// 	VkDeviceSize alignedOffset;
// 	friend class Allocation;
// 	friend class Allocator;
// };
// DEFINE_REF(SubAllocation)
// class Allocation
// {
// public:
// 	Allocation(PGraphics graphics, PAllocator pool, VkDeviceSize size, uint8 memoryTypeIndex,
// 			   VkMemoryPropertyFlags properties, VkMemoryDedicatedAllocateInfo *dedicatedInfo = nullptr);
// 	Allocation(const Allocation& other) = delete;
// 	Allocation& operator=(const Allocation& other) = delete;
// 	~Allocation();
// 	OSubAllocation getSuballocation(VkDeviceSize size, VkDeviceSize alignment);
// 	void markFree(PSubAllocation alloc);
// 	constexpr VkDeviceMemory getHandle() const
// 	{
// 		return allocatedMemory;
// 	}
// 	constexpr VkMemoryPropertyFlags getProperties() const
// 	{
// 		return properties;
// 	}
// 	constexpr void* map()
// 	{
// 		if (!canMap)
// 		{
// 			return nullptr;
// 		}
// 		if (!isMapped)
// 		{
// 			vkMapMemory(device, allocatedMemory, 0, bytesAllocated, 0, &mappedPointer);
// 			isMapped = true;
// 		}
// 		return mappedPointer;
// 	}

// 	void flushMemory();
// 	void invalidate();

// private:
// 	VkDevice device;
// 	PAllocator pool;
// 	VkDeviceSize bytesAllocated;
// 	VkDeviceSize bytesUsed;
// 	VkDeviceMemory allocatedMemory;
// 	Array<PSubAllocation> activeAllocations;
// 	Map<VkDeviceSize, VkDeviceSize> freeRanges;
// 	void *mappedPointer;
// 	uint8 canMap : 1;
// 	uint8 isMapped : 1;
// 	uint8 isDedicated : 1;
// 	VkMemoryPropertyFlags properties;
// 	uint8 memoryTypeIndex;
// 	friend class Allocator;
// };
// DEFINE_REF(Allocation)

// class Allocator
// {
// public:
// 	Allocator(PGraphics graphics);
// 	~Allocator();
// 	OSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
// 							VkMemoryDedicatedAllocateInfo *dedicatedInfo = nullptr);
// 	OSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
// 								   VkBuffer buffer)
// 	{
// 		VkMemoryDedicatedAllocateInfo allocInfo = {
// 			.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
// 			.pNext = nullptr,
// 			.image = VK_NULL_HANDLE,
// 			.buffer = buffer,
// 		};
// 		return allocate(requirements, props, &allocInfo);
// 	}
// 	OSubAllocation allocate(const VkMemoryRequirements2 &requirements, VkMemoryPropertyFlags props,
// 								   VkImage image)
// 	{
// 		VkMemoryDedicatedAllocateInfo allocInfo = {
// 			.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
// 			.pNext = nullptr,
// 			.image = image,
// 			.buffer = VK_NULL_HANDLE,
// 		};
// 		return allocate(requirements, props, &allocInfo);
// 	}

// 	void free(PAllocation allocation);
// 	void print();
// private:
// 	static constexpr VkDeviceSize DEFAULT_ALLOCATION = 16 * 1024 * 1024; // 16MB
// 	struct HeapInfo
// 	{
// 		VkDeviceSize maxSize = 0;
// 		VkDeviceSize inUse = 0;
// 		Array<OAllocation> allocations;
// 		HeapInfo() = default;
// 		HeapInfo(const HeapInfo& other) = delete;
// 		HeapInfo(HeapInfo&& other) = default;
// 		HeapInfo& operator=(const HeapInfo& other) = delete;
// 		HeapInfo& operator=(HeapInfo&& other) = default;
// 	};
// 	Array<HeapInfo> heaps;
// 	uint32 findMemoryType(uint32 typeBits, VkMemoryPropertyFlags properties);
// 	PGraphics graphics;
// 	VkPhysicalDeviceMemoryProperties memProperties;
// };
// DEFINE_REF(Allocator)
// class StagingBuffer
// {
// public:
// 	StagingBuffer(PGraphics graphics, OSubAllocation allocation, VkBuffer buffer, VkDeviceSize size, Gfx::QueueType owner);
// 	~StagingBuffer();
// 	void* map();
// 	void flush();
// 	void invalidate();
// 	constexpr VkBuffer getHandle() const
// 	{
// 		return buffer;
// 	}
// 	VkDeviceMemory getMemory() const;
// 	VkDeviceSize getOffset() const;
// 	constexpr uint64 getSize() const
// 	{
// 		return size;
// 	}
// 	constexpr Gfx::QueueType getOwner() const
// 	{
// 		return owner;
// 	}
// private:
// 	Gfx::QueueType owner;
// 	OSubAllocation allocation;
// 	PGraphics graphics;
// 	VkBuffer buffer;
// 	VkDeviceSize size;
// };
// DEFINE_REF(StagingBuffer)

// class StagingManager
// {
// public:
// 	StagingManager(PGraphics graphics, PAllocator pool);
// 	~StagingManager();
// 	OStagingBuffer create(uint64 size, Gfx::QueueType owner);
// 	void release(OStagingBuffer buffer);
// private:
// 	PGraphics graphics;
// 	PAllocator pool;
// 	Array<OStagingBuffer> freeBuffers;
// };
// DEFINE_REF(StagingManager)
// } // namespace Vulkan
// } // namespace Seele