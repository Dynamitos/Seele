#pragma once
#include "MinimalEngine.h"
#include "VulkanGraphicsEnums.h"
#include "Containers/Map.h"
#include "Containers/Array.h"

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
			SubAllocation(Allocation* owner, uint32 allocatedOffset, uint32 size, uint32 alignedOffset, uint32 allocatedSize);
		private:
			Allocation* owner;
			uint32 allocatedOffset;
			uint32 size;
			uint32 alignedOffset;
			uint32 allocatedSize;
			friend class Allocation;
			friend class Allocator;
		};
		DEFINE_REF(SubAllocation);
		class Allocation
		{
		public:
			Allocation(WGraphics graphics, Allocator* allocator, uint32 size, uint32 memoryTypeIndex, VkMemoryPropertyFlags properties, bool isDedicated);
			PSubAllocation getSuballocation(uint32 size, uint32 alignment);
		private:
			Allocator* allocator;
			VkDevice device;
			VkDeviceMemory allocatedMemory;
			VkDeviceSize bytesAllocated;
			VkDeviceSize bytesUsed;
			VkMemoryPropertyFlags properties;
			Array<PSubAllocation> activeAllocations;
			Array<PSubAllocation> freeRanges;
			void* mappedPointer;
			uint8 isDedicated : 1;
			uint8 canMap : 1;
			uint8 memoryTypeIndex;
			friend class Allocator;
		};
		DEFINE_REF(Allocation);

		class Allocator
		{
		public:
			Allocator(WGraphics graphics);
			~Allocator();
			PSubAllocation allocate(uint64 size, const VkMemoryRequirements2& requirements, VkMemoryPropertyFlags props);

		private:
			enum
			{
				MemoryBlockSize = 64 * 1024 * 1024 // 64MB
			};
			struct HeapInfo
			{
				uint32 maxSize = 0;
				Array<PAllocation> allocations;
			};
			Array<HeapInfo> heaps;
			VkResult findMemoryType(uint32 typeBits, VkMemoryPropertyFlags properties, uint32* typeIndex);
			WGraphics graphics;
			VkPhysicalDeviceMemoryProperties memProperties;
		};
		DEFINE_REF(Allocator);
	}
}