#pragma once
#include "Containers/Array.h"
#include <vulkan/vulkan.h>

namespace Seele {
namespace Vulkan {
VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT pCallback);
} // namespace Vulkan
} // namespace Seele