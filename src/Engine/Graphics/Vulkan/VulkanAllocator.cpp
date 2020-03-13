#include "VulkanAllocator.h"
#include "VulkanGraphics.h"
Seele::VulkanAllocator::VulkanAllocator(PVulkanGraphics graphics)
	: graphics(graphics)
{
	vkGetPhysicalDeviceMemoryProperties(graphics->getPhysicalDevice(), &memProperties);
	heaps.resize(memProperties.memoryHeapCount);
	for (size_t i = 0; i < memProperties.memoryHeapCount; i++)
	{
		std::cout << "Heap: Flags: " << memProperties.memoryHeaps[i].flags << " Size: " << memProperties.memoryHeaps[i].size << std::endl;
	}
	for (size_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		VkMemoryType& type = memProperties.memoryTypes[i];
		std::cout << "Memory Type: Properties" << type.propertyFlags << " on Index: " << type.heapIndex << std::endl;
	}
}

Seele::VulkanAllocator::~VulkanAllocator()
{
}

Seele::PVulkanSubAllocation Seele::VulkanAllocator::allocate(uint64 size, VkMemoryPropertyFlags properties)
{
	return PVulkanSubAllocation();
}
