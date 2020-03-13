#include "VulkanGraphicsResources.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"

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

Seele::VulkanPipelineLayout::~VulkanPipelineLayout()
{
	if (layoutHandle != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(graphics->getDevice(), layoutHandle, nullptr);
	}
}

void Seele::VulkanPipelineLayout::create()
{
	vulkanDescriptorLayouts.resize(descriptorSetLayouts.size());
	for (size_t i = 0; i < descriptorSetLayouts.size(); ++i)
	{
		PVulkanDescriptorLayout layout = descriptorSetLayouts[i].cast<VulkanDescriptorLayout>();
		layout->create();
		vulkanDescriptorLayouts[i] = layout->getHandle();
	}
	VkPipelineLayoutCreateInfo createInfo =
		init::PipelineLayoutCreateInfo(vulkanDescriptorLayouts.data(), vulkanDescriptorLayouts.size());
	Array<VkPushConstantRange> vkPushConstants(pushConstants.size());
	for (size_t i = 0; i < pushConstants.size(); i++)
	{
		vkPushConstants[i].offset = pushConstants[i].offset;
		vkPushConstants[i].size = pushConstants[i].size;
		vkPushConstants[i].stageFlags = cast((VkShaderStageFlagBits)pushConstants[i].stageFlags);
	}
	createInfo.pushConstantRangeCount = vkPushConstants.size();
	createInfo.pPushConstantRanges = vkPushConstants.data();
	VK_CHECK(vkCreatePipelineLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));

	layoutHash = memCrc32(&createInfo, sizeof(VkPipelineLayoutCreateInfo), 0);
}

Seele::VulkanDescriptorSet::~VulkanDescriptorSet()
{
}

void Seele::VulkanDescriptorSet::updateBuffer(uint32_t binding, PUniformBuffer uniformBuffer)
{
	PVulkanUniformBuffer vulkanBuffer = uniformBuffer.cast<VulkanUniformBuffer>();
//	VkDescriptorBufferInfo bufferInfo = init::DescriptorBufferInfo(vulkanBuffer->getHandle(), vulkanBuffer->getOffset(), vulkanBuffer->getSize());
//	bufferInfos.add(bufferInfo);

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, &bufferInfos.back());
	writeDescriptors.add(writeDescriptor);
}

void Seele::VulkanDescriptorSet::updateBuffer(uint32_t binding, PStructuredBuffer uniformBuffer)
{
	PVulkanStructuredBuffer vulkanBuffer = uniformBuffer.cast<VulkanStructuredBuffer>();
//	VkDescriptorBufferInfo bufferInfo = init::DescriptorBufferInfo(vulkanBuffer->getHandle(), vulkanBuffer->getOffset(), vulkanBuffer->getSize());
//	bufferInfos.add(bufferInfo);

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding, &bufferInfos.back());
	writeDescriptors.add(writeDescriptor);
}

void Seele::VulkanDescriptorSet::updateSampler(uint32_t binding, PSamplerState samplerState)
{
	PVulkanSamplerState vulkanSampler = samplerState.cast<VulkanSamplerState>();
	VkDescriptorImageInfo imageInfo =
		init::DescriptorImageInfo(
			vulkanSampler->sampler,
			VK_NULL_HANDLE,
			VK_IMAGE_LAYOUT_BEGIN_RANGE);
	imageInfos.add(imageInfo);

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle, VK_DESCRIPTOR_TYPE_SAMPLER, binding, &imageInfos.back());
	writeDescriptors.add(writeDescriptor);
}

void Seele::VulkanDescriptorSet::updateTexture(uint32_t binding, PTexture texture, PSamplerState samplerState)
{
//	VulkanTextureBase* vulkanTexture = VulkanTextureBase::cast(texture);
	//It is assumed that the image is in the correct layout
//	VkDescriptorImageInfo imageInfo =
//		init::DescriptorImageInfo(
//			VK_NULL_HANDLE,
//			vulkanTexture->defaultView.view,
//			graphics->getRHIDevice().findLayout(vulkanTexture->surface.image));
	if (samplerState != nullptr)
	{
//		PVulkanSamplerState vulkanSampler = samplerState.cast<VulkanSamplerState>();
//		imageInfo.sampler = vulkanSampler->sampler;
	}
//	imageInfos.add(imageInfo);
	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle, samplerState != nullptr ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, binding, &imageInfos.back());
//	if (vulkanTexture->surface.usageFlags & TexCreate_UAV)
	{
//		writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	}
	writeDescriptors.add(writeDescriptor);
}

bool Seele::VulkanDescriptorSet::operator<(PDescriptorSet other)
{
	PVulkanDescriptorSet otherSet = other.cast<VulkanDescriptorSet>();
	return setHandle < otherSet->setHandle;
}

void Seele::VulkanDescriptorSet::writeChanges()
{
	if (writeDescriptors.size() > 0)
	{
		vkUpdateDescriptorSets(graphics->getDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		writeDescriptors.clear();
		imageInfos.clear();
		bufferInfos.clear();
	}
}

Seele::VulkanDescriptorAllocator::VulkanDescriptorAllocator(PVulkanGraphics graphics, VulkanDescriptorLayout& layout)
	: layout(layout)
	, graphics(graphics)
{
	uint32 perTypeSizes[VK_DESCRIPTOR_TYPE_END_RANGE];
	std::memset(perTypeSizes, 0, sizeof(perTypeSizes));
	for (int i = 0; i < layout.getBindings().size(); ++i)
	{
		auto& binding = layout.getBindings()[i];
		int typeIndex = binding.descriptorType;
		perTypeSizes[typeIndex] += 256;
	}
	Array<VkDescriptorPoolSize> poolSizes;
	for (int i = 0; i < VK_DESCRIPTOR_TYPE_END_RANGE; ++i)
	{
		if (perTypeSizes[i] > 0)
		{
			VkDescriptorPoolSize size;
			size.descriptorCount = perTypeSizes[i];
			size.type = (VkDescriptorType)i;
			poolSizes.add(size);
		}
	}
	VkDescriptorPoolCreateInfo createInfo = init::DescriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), maxSets);
	VK_CHECK(vkCreateDescriptorPool(graphics->getDevice(), &createInfo, nullptr, &poolHandle));
}

Seele::VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
{
	vkDestroyDescriptorPool(graphics->getDevice(), poolHandle, nullptr);
}

void Seele::VulkanDescriptorAllocator::allocateDescriptorSet(PDescriptorSet& descriptorSet)
{
	descriptorSet = new VulkanDescriptorSet(graphics, this);
	PVulkanDescriptorSet vulkanSet = descriptorSet.cast<VulkanDescriptorSet>();
	VkDescriptorSetLayout layoutHandle = layout.getHandle();
	VkDescriptorSetAllocateInfo allocInfo =
		init::DescriptorSetAllocateInfo(poolHandle, &layoutHandle, 1);
	VK_CHECK(vkAllocateDescriptorSets(graphics->getDevice(), &allocInfo, &vulkanSet->setHandle));
}
