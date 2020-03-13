#pragma once
#include "MinimalEngine.h"
#include "VulkanGraphicsEnums.h"
#include "Containers/Map.h"
#include "Containers/Array.h"

namespace Seele
{
	DECLARE_REF(VulkanGraphics);
	class VulkanSubAllocation
	{

	};
	DEFINE_REF(VulkanSubAllocation);
	class VulkanAllocation
	{
	private:
		VkDeviceMemory allocatedMemory;
		VkDeviceSize bytesAllocated;
		VkDeviceSize bytesUsed;
		uint8 isDedicated;
	};
	DEFINE_REF(VulkanAllocation);

	class VulkanAllocator
	{
	public:
		VulkanAllocator(PVulkanGraphics graphics);
		~VulkanAllocator();
		PVulkanSubAllocation allocate(uint64 size, VkMemoryPropertyFlags properties);
	private:
		struct HeapInfo
		{
			uint32 maxSize = 0;
			Array<VulkanAllocation> allocations;
			Array<VkMemoryType> types;
		};
		Array<HeapInfo> heaps;
		PVulkanGraphics graphics;
		VkPhysicalDeviceMemoryProperties memProperties;
	};
	DEFINE_REF(VulkanAllocator);
}