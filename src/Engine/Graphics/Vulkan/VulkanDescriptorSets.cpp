#include "VulkanDescriptorSets.h"
#include "VulkanGraphicsResources.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"

using namespace Seele;
using namespace Seele::Vulkan;

DescriptorLayout::DescriptorLayout(PGraphics graphics) 
	: graphics(graphics)
	, layoutHandle(VK_NULL_HANDLE)
{
}
DescriptorLayout::~DescriptorLayout()
{
	if (layoutHandle != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(graphics->getDevice(), layoutHandle, nullptr);
	}
}

void DescriptorLayout::create()
{
	if (layoutHandle != VK_NULL_HANDLE)
	{
		return;
	}
	bindings.resize(descriptorBindings.size());
	for (size_t i = 0; i < descriptorBindings.size(); ++i)
	{
		VkDescriptorSetLayoutBinding &binding = bindings[i];
		const Gfx::DescriptorBinding &rhiBinding = descriptorBindings[i];
		binding.binding = rhiBinding.binding;
		binding.descriptorCount = rhiBinding.descriptorCount;
		binding.descriptorType = cast(rhiBinding.descriptorType);
		binding.stageFlags = rhiBinding.shaderStages;
		binding.pImmutableSamplers = nullptr;
	}
	VkDescriptorSetLayoutCreateInfo createInfo =
		init::DescriptorSetLayoutCreateInfo(bindings.data(), bindings.size());
	VK_CHECK(vkCreateDescriptorSetLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));

	std::cout << "creating descriptorlayout " << layoutHandle << std::endl;

	allocator = new DescriptorAllocator(graphics, *this);

	boost::crc_32_type result;
	result.process_bytes(bindings.data(), sizeof(VkDescriptorSetLayoutBinding) * bindings.size());
	hash = result.checksum();
}

PipelineLayout::~PipelineLayout()
{
	if (layoutHandle != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(graphics->getDevice(), layoutHandle, nullptr);
	}
}

static Map<uint32, VkPipelineLayout> layoutCache;

void PipelineLayout::create()
{
	vulkanDescriptorLayouts.resize(descriptorSetLayouts.size());
	for (size_t i = 0; i < descriptorSetLayouts.size(); ++i)
	{
		// There could be unused descriptor set indices
		if(descriptorSetLayouts[i] == nullptr)
		{
			continue;
		}
		PDescriptorLayout layout = descriptorSetLayouts[i].cast<DescriptorLayout>();
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

	boost::crc_32_type result;
	result.process_bytes(createInfo.pPushConstantRanges, sizeof(VkPushConstantRange) * createInfo.pushConstantRangeCount);
	result.process_bytes(createInfo.pSetLayouts, sizeof(VkDescriptorSetLayout) * createInfo.setLayoutCount);
	layoutHash = result.checksum();

	if(layoutCache[layoutHash] != VK_NULL_HANDLE)
	{
		layoutHandle = layoutCache[layoutHash];
		return;
	}

	VK_CHECK(vkCreatePipelineLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));
	layoutCache[layoutHash] = layoutHandle;
}

void PipelineLayout::reset()
{
	vkDestroyPipelineLayout(graphics->getDevice(), layoutHandle, nullptr);
	descriptorSetLayouts.clear();
	pushConstants.clear();
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::beginFrame() 
{
	currentFrameSet = (currentFrameSet + 1) % Gfx::numFramesBuffered;
}

void DescriptorSet::endFrame() 
{
}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer)
{
	PUniformBuffer vulkanBuffer = uniformBuffer.cast<UniformBuffer>();
	UniformBuffer* cachedBuffer = reinterpret_cast<UniformBuffer*>(cachedData[currentFrameSet][binding]);
	if(vulkanBuffer->isDataEquals(cachedBuffer))
	{
		return;
	}
	VkDescriptorBufferInfo bufferInfo = init::DescriptorBufferInfo(vulkanBuffer->getHandle(), 0, vulkanBuffer->getSize());
	bufferInfos.add(bufferInfo);

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle[currentFrameSet], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, &bufferInfos.back());
	writeDescriptors.add(writeDescriptor);

	cachedData[currentFrameSet][binding] = vulkanBuffer.getHandle();
}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PStructuredBuffer uniformBuffer)
{
	PStructuredBuffer vulkanBuffer = uniformBuffer.cast<StructuredBuffer>();
	StructuredBuffer* cachedBuffer = reinterpret_cast<StructuredBuffer*>(cachedData[currentFrameSet][binding]);
	if(vulkanBuffer.getHandle() == cachedBuffer)
	{
		return;
	}
	VkDescriptorBufferInfo bufferInfo = init::DescriptorBufferInfo(vulkanBuffer->getHandle(), 0, vulkanBuffer->getSize());
	bufferInfos.add(bufferInfo);

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle[currentFrameSet], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding, &bufferInfos.back());
	writeDescriptors.add(writeDescriptor);

	cachedData[currentFrameSet][binding] = vulkanBuffer.getHandle();
}

void DescriptorSet::updateSampler(uint32_t binding, Gfx::PSamplerState samplerState)
{
	PSamplerState vulkanSampler = samplerState.cast<SamplerState>();
	SamplerState* cachedSampler = reinterpret_cast<SamplerState*>(cachedData[currentFrameSet][binding]);
	if(vulkanSampler.getHandle() == cachedSampler)
	{
		return;
	}
	VkDescriptorImageInfo imageInfo =
		init::DescriptorImageInfo(
			vulkanSampler->sampler,
			VK_NULL_HANDLE,
			VK_IMAGE_LAYOUT_UNDEFINED);
	imageInfos.add(imageInfo);

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle[currentFrameSet], VK_DESCRIPTOR_TYPE_SAMPLER, binding, &imageInfos.back());
	writeDescriptors.add(writeDescriptor);

	cachedData[currentFrameSet][binding] = vulkanSampler.getHandle();
}

void DescriptorSet::updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSamplerState samplerState)
{
	TextureHandle* vulkanTexture = TextureBase::cast(texture);
	TextureHandle* cachedTexture = reinterpret_cast<TextureHandle*>(cachedData[currentFrameSet][binding]);
	if(vulkanTexture == cachedTexture)
	{
		return;
	}
	//It is assumed that the image is in the correct layout
	VkDescriptorImageInfo imageInfo =
		init::DescriptorImageInfo(
			VK_NULL_HANDLE,
			vulkanTexture->getView(),
			vulkanTexture->getLayout());
	if (samplerState != nullptr)
	{
		PSamplerState vulkanSampler = samplerState.cast<SamplerState>();
		imageInfo.sampler = vulkanSampler->sampler;
	}
	imageInfos.add(imageInfo);
	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle[currentFrameSet], samplerState != nullptr ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, binding, &imageInfos.back());
	if (vulkanTexture->getUsage() & VK_IMAGE_USAGE_STORAGE_BIT)
	{
		writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	}
	writeDescriptors.add(writeDescriptor);

	cachedData[currentFrameSet][binding] = vulkanTexture;
}

bool DescriptorSet::operator<(Gfx::PDescriptorSet other)
{
	PDescriptorSet otherSet = other.cast<DescriptorSet>();
	return setHandle < otherSet->setHandle;
}

void DescriptorSet::writeChanges()
{
	if (writeDescriptors.size() > 0)
	{
		vkUpdateDescriptorSets(graphics->getDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		writeDescriptors.clear();
		imageInfos.clear();
		bufferInfos.clear();
	}
}

DescriptorAllocator::DescriptorAllocator(PGraphics graphics, DescriptorLayout &layout)
	: layout(layout), graphics(graphics), currentCachedIndex(0)
{
	std::memset(&cachedHandles, 0, sizeof(cachedHandles));

	uint32 perTypeSizes[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT]; // TODO: FIX ENUM
	std::memset(perTypeSizes, 0, sizeof(perTypeSizes));
	for (uint32 i = 0; i < layout.getBindings().size(); ++i)
	{
		auto &binding = layout.getBindings()[i];
		int typeIndex = binding.descriptorType;
		perTypeSizes[typeIndex] += 256;
	}
	Array<VkDescriptorPoolSize> poolSizes;
	for (uint32 i = 0; i < VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; ++i)
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

DescriptorAllocator::~DescriptorAllocator()
{
	vkDestroyDescriptorPool(graphics->getDevice(), poolHandle, nullptr);
	graphics = nullptr;
}

void DescriptorAllocator::allocateDescriptorSet(Gfx::PDescriptorSet &descriptorSet)
{
	descriptorSet = new DescriptorSet(graphics, this);
	PDescriptorSet vulkanSet = descriptorSet.cast<DescriptorSet>();
	VkDescriptorSetLayout layoutHandle = layout.getHandle();
	VkDescriptorSetAllocateInfo allocInfo =
		init::DescriptorSetAllocateInfo(poolHandle, &layoutHandle, 1);
	for(uint32 i = 0; i < Gfx::numFramesBuffered; ++i)
	{
		if(cachedHandles[currentCachedIndex] != VK_NULL_HANDLE)
		{
			vulkanSet->setHandle[i] = cachedHandles[currentCachedIndex++];
		}
		else
		{
			VK_CHECK(vkAllocateDescriptorSets(graphics->getDevice(), &allocInfo, &vulkanSet->setHandle[i]));
			cachedHandles[currentCachedIndex++] = vulkanSet->setHandle[i];
		}
		
		vulkanSet->cachedData[i].resize(layout.bindings.size());
		// Not really pretty, but this way the set knows which ones are valid
		std::memset(vulkanSet->cachedData[i].data(), 0, sizeof(void*) * vulkanSet->cachedData[i].size());
	}
}

void DescriptorAllocator::reset()
{
	currentCachedIndex = 0;
}