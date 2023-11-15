#pragma once
#include "Containers/Array.h"
#include <vulkan/vulkan.h>

namespace Seele
{
namespace Vulkan
{
VkBool32 debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData);

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);
void DestroyDebugReportCallbackEXT(VkInstance instance, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT pCallback);
} // namespace Vulkan
} // namespace Seele