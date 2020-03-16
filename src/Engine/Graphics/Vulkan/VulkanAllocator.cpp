#include "VulkanAllocator.h"
#include "VulkanGraphics.h"
Seele::VulkanAllocator::VulkanAllocator(PVulkanGraphics graphics)
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
	for (size_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		VkMemoryType& type = memProperties.memoryTypes[i];
		HeapInfo& heapInfo = heaps[type.heapIndex];
		heapInfo.types.add(type);
	}
}

Seele::VulkanAllocator::~VulkanAllocator()
{
}

Seele::PVulkanSubAllocation Seele::VulkanAllocator::allocate(uint64 size, VkMemoryPropertyFlags properties)
{
	return PVulkanSubAllocation();
}
