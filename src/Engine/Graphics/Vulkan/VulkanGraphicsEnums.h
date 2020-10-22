#pragma once
#include "Graphics/GraphicsEnums.h"
#include <vulkan/vulkan.h>
#include <iostream>

#define VK_CHECK(f)                                                                                                       \
	{                                                                                                                     \
		VkResult res = (f);                                                                                               \
		if (res != VK_SUCCESS)                                                                                            \
		{                                                                                                                 \
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
	CONTROL = 1,
	EVALUATION = 2,
	GEOMETRY = 3,
	FRAGMENT = 4,
	COMPUTE = 5,
};

VkDescriptorType cast(const Gfx::SeDescriptorType &descriptorType);
Gfx::SeDescriptorType cast(const VkDescriptorType &descriptorType);
VkShaderStageFlagBits cast(const Gfx::SeShaderStageFlagBits &stage);
Gfx::SeShaderStageFlagBits cast(const VkShaderStageFlagBits &stage);
VkFormat cast(const Gfx::SeFormat &format);
Gfx::SeFormat cast(const VkFormat &format);
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
} // namespace Vulkan
} // namespace Seele