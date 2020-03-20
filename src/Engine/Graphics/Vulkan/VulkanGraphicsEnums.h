#pragma once
#include "Graphics/GraphicsEnums.h"
#include <vulkan/vulkan.h>

#define VK_CHECK(f)																						\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << res << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}

namespace Seele
{
	namespace Vulkan
	{
		VkDescriptorType cast(const Gfx::SeDescriptorType& descriptorType);
		Gfx::SeDescriptorType cast(const VkDescriptorType& descriptorType);
		VkShaderStageFlagBits cast(const Gfx::SeShaderStageFlagBits& stage);
		Gfx::SeShaderStageFlagBits cast(const VkShaderStageFlagBits& stage);
	}
}