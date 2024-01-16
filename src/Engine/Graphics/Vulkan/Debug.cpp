#include "Debug.h"
#include <iostream>

using namespace Seele::Vulkan;

VkBool32 Seele::Vulkan::debugCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char *layerPrefix, const char *msg, void *)
{
	std::cerr << layerPrefix << ": " << msg << std::endl;
	return VK_FALSE;
}

VkResult Seele::Vulkan::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void Seele::Vulkan::DestroyDebugReportCallbackEXT(VkInstance instance, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT pCallback)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr)
	{
		func(instance, pCallback, pAllocator);
	}
}
