#include "VulkanGraphicsResources.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"
#include <iostream>

Seele::VulkanDescriptorLayout::~VulkanDescriptorLayout()
{
	if (layoutHandle != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(graphics->getDevice(), layoutHandle, nullptr);
	}
}

void Seele::VulkanDescriptorLayout::create()
{
	if (layoutHandle != VK_NULL_HANDLE)
	{
		return;
	}
	bindings.resize(descriptorBindings.size());
	for (size_t i = 0; i < descriptorBindings.size(); ++i)
	{
		VkDescriptorSetLayoutBinding& binding = bindings[i];
		const DescriptorBinding& rhiBinding = descriptorBindings[i];
		binding.binding = rhiBinding.binding;
		binding.descriptorCount = rhiBinding.descriptorCount;
		binding.descriptorType = cast(rhiBinding.descriptorType);
		binding.stageFlags = rhiBinding.shaderStages;
		binding.pImmutableSamplers = nullptr;
	}
	VkDescriptorSetLayoutCreateInfo createInfo =
		init::DescriptorSetLayoutCreateInfo(bindings.data(), bindings.size());
	VK_CHECK(vkCreateDescriptorSetLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));

	allocator = new VulkanDescriptorAllocator(graphics, *this);
}

void Seele::VulkanPipelineLayout::create()
{
	vulkanDescriptorLayouts.resize(descriptorSetLayouts.size());
	for (size_t i = 0; i < descriptorSetLayouts.size(); ++i)
	{
		PVulkanDescriptorLayout layout = descriptorSetLayouts[i].cast<VulkanDescriptorLayout>();
	}
}