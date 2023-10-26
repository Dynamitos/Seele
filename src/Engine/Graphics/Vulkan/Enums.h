#pragma once
#include "Graphics/Enums.h"
#include <vulkan/vulkan.h>
#include <iostream>

#define VK_CHECK(f)                                                                                                       \
	{                                                                                                                     \
		VkResult res = (f);                                                                                               \
		if (res != VK_SUCCESS)                                                                                            \
		{                                                                                                                 \
			if(res == VK_ERROR_DEVICE_LOST)                                                                               \
			{                                                                                                             \
				std::this_thread::sleep_for(std::chrono::seconds(3));                                                     \
			}                                                                                                             \
			std::cout << "Fatal : VkResult is \"" << res << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
			assert(res == VK_SUCCESS);                                                                                    \
		}                                                                                                                 \
	}

namespace Seele
{
namespace Vulkan
{

enum class ShaderType
{
	VERTEX = 0,
	FRAGMENT = 1,
	COMPUTE = 2,
	TASK = 3,
	MESH = 4,
};

VkDescriptorType cast(const Gfx::SeDescriptorType &descriptorType);
Gfx::SeDescriptorType cast(const VkDescriptorType &descriptorType);
VkShaderStageFlagBits cast(const Gfx::SeShaderStageFlagBits &stage);
Gfx::SeShaderStageFlagBits cast(const VkShaderStageFlagBits &stage);
VkFormat cast(const Gfx::SeFormat &format);
Gfx::SeFormat cast(const VkFormat &format);
VkImageLayout cast(const Gfx::SeImageLayout &imageLayout);
Gfx::SeImageLayout cast(const VkImageLayout &imageLayout);
VkAttachmentStoreOp cast(const Gfx::SeAttachmentStoreOp &storeOp);
Gfx::SeAttachmentStoreOp cast(const VkAttachmentStoreOp &storeOp);
VkAttachmentLoadOp cast(const Gfx::SeAttachmentLoadOp &loadOp);
Gfx::SeAttachmentLoadOp cast(const VkAttachmentLoadOp &loadOp);
VkIndexType cast(const Gfx::SeIndexType &indexType);
Gfx::SeIndexType cast(const VkIndexType &indexType);
VkPrimitiveTopology cast(const Gfx::SePrimitiveTopology &topology);
Gfx::SePrimitiveTopology cast(const VkPrimitiveTopology &topology);
VkPolygonMode cast(const Gfx::SePolygonMode &mode);
Gfx::SePolygonMode cast(const VkPolygonMode &mode);
VkCompareOp cast(const Gfx::SeCompareOp &op);
Gfx::SeCompareOp cast(const VkCompareOp &op);
VkClearValue cast(const Gfx::SeClearValue &clear);
Gfx::SeClearValue cast(const VkClearValue &clear);
VkSamplerAddressMode cast(const Gfx::SeSamplerAddressMode &mode);
Gfx::SeSamplerAddressMode cast(const VkSamplerAddressMode &mode);
VkBorderColor cast(const Gfx::SeBorderColor &color);
Gfx::SeBorderColor cast(const VkBorderColor &color);
VkFilter cast(const Gfx::SeFilter &filter);
Gfx::SeFilter cast(const VkFilter &filter);
VkSamplerMipmapMode cast(const Gfx::SeSamplerMipmapMode &filter);
Gfx::SeSamplerMipmapMode cast(const VkSamplerMipmapMode &filter);
} // namespace Vulkan
} // namespace Seele