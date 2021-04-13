#include "VulkanDescriptorSets.h"
#include "VulkanGraphicsResources.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"
#include "VulkanCommandBuffer.h"

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
		init::DescriptorSetLayoutCreateInfo(bindings.data(), (uint32)bindings.size());
	VK_CHECK(vkCreateDescriptorSetLayout(graphics->getDevice(), &createInfo, nullptr, &layoutHandle));

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
		init::PipelineLayoutCreateInfo(vulkanDescriptorLayouts.data(), (uint32)vulkanDescriptorLayouts.size());
	Array<VkPushConstantRange> vkPushConstants(pushConstants.size());
	for (size_t i = 0; i < pushConstants.size(); i++)
	{
		vkPushConstants[i].offset = pushConstants[i].offset;
		vkPushConstants[i].size = pushConstants[i].size;
		vkPushConstants[i].stageFlags = cast((VkShaderStageFlagBits)pushConstants[i].stageFlags);
	}
	createInfo.pushConstantRangeCount = (uint32)vkPushConstants.size();
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

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer)
{
	PUniformBuffer vulkanBuffer = uniformBuffer.cast<UniformBuffer>();
	UniformBuffer* cachedBuffer = reinterpret_cast<UniformBuffer*>(cachedData[Gfx::currentFrameIndex][binding]);
	if(vulkanBuffer->isDataEquals(cachedBuffer))
	{
		return;
	}
	bufferInfos.add(init::DescriptorBufferInfo(vulkanBuffer->getHandle(), 0, vulkanBuffer->getSize()));

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle[Gfx::currentFrameIndex], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, &bufferInfos.back());
	writeDescriptors.add(writeDescriptor);

	cachedData[Gfx::currentFrameIndex][binding] = vulkanBuffer.getHandle();
}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PStructuredBuffer uniformBuffer)
{
	PStructuredBuffer vulkanBuffer = uniformBuffer.cast<StructuredBuffer>();
	StructuredBuffer* cachedBuffer = reinterpret_cast<StructuredBuffer*>(cachedData[Gfx::currentFrameIndex][binding]);
	if(vulkanBuffer.getHandle() == cachedBuffer)
	{
		return;
	}
	bufferInfos.add(init::DescriptorBufferInfo(vulkanBuffer->getHandle(), 0, vulkanBuffer->getSize()));

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle[Gfx::currentFrameIndex], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, binding, &bufferInfos.back());
	writeDescriptors.add(writeDescriptor);

	cachedData[Gfx::currentFrameIndex][binding] = vulkanBuffer.getHandle();
}

void DescriptorSet::updateSampler(uint32_t binding, Gfx::PSamplerState samplerState)
{
	PSamplerState vulkanSampler = samplerState.cast<SamplerState>();
	SamplerState* cachedSampler = reinterpret_cast<SamplerState*>(cachedData[Gfx::currentFrameIndex][binding]);
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

	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle[Gfx::currentFrameIndex], VK_DESCRIPTOR_TYPE_SAMPLER, binding, &imageInfos.back());
	writeDescriptors.add(writeDescriptor);

	cachedData[Gfx::currentFrameIndex][binding] = vulkanSampler.getHandle();
}

void DescriptorSet::updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSamplerState samplerState)
{
	TextureHandle* vulkanTexture = TextureBase::cast(texture);
	TextureHandle* cachedTexture = reinterpret_cast<TextureHandle*>(cachedData[Gfx::currentFrameIndex][binding]);
	if(vulkanTexture == cachedTexture)
	{
		std::cout << "Cached texture is same as new one, skipping update" << std::endl;
		return;
	}
	std::cout << "Texture changed, updating" << std::endl;
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
	VkWriteDescriptorSet writeDescriptor = init::WriteDescriptorSet(setHandle[Gfx::currentFrameIndex], samplerState != nullptr ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, binding, &imageInfos.back());
	if (vulkanTexture->getUsage() & VK_IMAGE_USAGE_STORAGE_BIT)
	{
		writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	}
	writeDescriptors.add(writeDescriptor);

	cachedData[Gfx::currentFrameIndex][binding] = vulkanTexture;
}

bool DescriptorSet::operator<(Gfx::PDescriptorSet other)
{
	PDescriptorSet otherSet = other.cast<DescriptorSet>();
	return this < otherSet.getHandle();
}

uint32 DescriptorSet::getSetIndex() const
{
	return owner->getLayout().getSetIndex();
}

void DescriptorSet::writeChanges()
{
	if (writeDescriptors.size() > 0)
	{
		assert(!isCurrentlyBound());
		vkUpdateDescriptorSets(graphics->getDevice(), (uint32)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
		writeDescriptors.clear();
		imageInfos.clear();
		bufferInfos.clear();
	}
}

DescriptorAllocator::DescriptorAllocator(PGraphics graphics, DescriptorLayout &layout)
	: graphics(graphics)
	, layout(layout)
{
	for(uint32 i = 0; i < cachedHandles.size(); ++i)
	{
		cachedHandles[i] = new DescriptorSet(graphics, this);
	}

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
	VkDescriptorPoolCreateInfo createInfo = init::DescriptorPoolCreateInfo((uint32)poolSizes.size(), poolSizes.data(), maxSets * Gfx::numFramesBuffered);
	VK_CHECK(vkCreateDescriptorPool(graphics->getDevice(), &createInfo, nullptr, &poolHandle));
}

DescriptorAllocator::~DescriptorAllocator()
{
	vkDestroyDescriptorPool(graphics->getDevice(), poolHandle, nullptr);
	graphics = nullptr;
}

void DescriptorAllocator::allocateDescriptorSet(Gfx::PDescriptorSet &descriptorSet)
{
	VkDescriptorSetLayout layoutHandle = layout.getHandle();
	VkDescriptorSetLayout layoutArray[Gfx::numFramesBuffered];
	for (uint32 i = 0; i < Gfx::numFramesBuffered; i++)
	{
		layoutArray[i] = layoutHandle;
	}
	
	VkDescriptorSetAllocateInfo allocInfo =
		init::DescriptorSetAllocateInfo(poolHandle, layoutArray, Gfx::numFramesBuffered);

	for(uint32 setIndex = 0; setIndex < cachedHandles.size(); ++setIndex)
	{
		if(cachedHandles[setIndex]->isCurrentlyBound() || cachedHandles[setIndex]->isCurrentlyInUse())
		{
			// Currently in use, skip
			continue;
		}
		if(cachedHandles[setIndex]->getHandle() == VK_NULL_HANDLE)
		{
			//If it hasnt been initialized, allocate it
			VK_CHECK(vkAllocateDescriptorSets(graphics->getDevice(), &allocInfo, cachedHandles[setIndex]->setHandle));
		}
		cachedHandles[setIndex]->currentlyInUse = true;
		descriptorSet = cachedHandles[setIndex];
		
		PDescriptorSet vulkanSet = descriptorSet.cast<DescriptorSet>();
		for(uint32 frameIndex = 0; frameIndex < Gfx::numFramesBuffered; ++frameIndex)
		{
			vulkanSet->cachedData[frameIndex].resize(layout.bindings.size());
			// Not really pretty, but this way the set knows which ones are valid
			std::memset(vulkanSet->cachedData[frameIndex].data(), 0, sizeof(void*) * vulkanSet->cachedData[frameIndex].size());
		}
		//Found set, stop searching
		return;
	}
	throw std::logic_error("Out of descriptor sets");
}

void DescriptorAllocator::reset()
{
	for(uint32 i = 0; i < cachedHandles.size(); ++i)
	{
		cachedHandles[i]->free();
	}
}