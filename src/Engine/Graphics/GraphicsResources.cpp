#include "GraphicsResources.h"

using namespace Seele::Gfx;

void DescriptorLayout::addDescriptorBinding(uint32 bindingIndex, SeDescriptorType type, uint32 arrayCount)
{
	if (descriptorBindings.size() <= bindingIndex)
	{
		descriptorBindings.resize(bindingIndex + 1);
	}
	DescriptorBinding binding;
	binding.binding = bindingIndex;
	binding.descriptorType = type;
	binding.descriptorCount = arrayCount;
	descriptorBindings[bindingIndex] = binding;
}

PDescriptorSet DescriptorLayout::allocatedDescriptorSet()
{
	PDescriptorSet result;
	allocator->allocateDescriptorSet(result);
	return result;
}

void PipelineLayout::addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout)
{
	if (descriptorSetLayouts.size() <= setIndex)
	{
		descriptorSetLayouts.resize(setIndex + 1);
	}
	if (descriptorSetLayouts[setIndex] != nullptr)
	{
		auto& thisBindings = descriptorSetLayouts[setIndex]->descriptorBindings;
		auto& otherBindings = layout->descriptorBindings;
		thisBindings.resize(otherBindings.size());
		for (size_t i = 0; i < otherBindings.size(); ++i)
		{
			if (otherBindings[i].descriptorType != SE_DESCRIPTOR_TYPE_MAX_ENUM)
			{
				assert(thisBindings[i].descriptorType != SE_DESCRIPTOR_TYPE_MAX_ENUM ? thisBindings[i].descriptorType == otherBindings[i].descriptorType : true);
				thisBindings[i] = otherBindings[i];
			}
		}
	}
	else
	{
		descriptorSetLayouts[setIndex] = layout;
	}
}

void PipelineLayout::addPushConstants(const SePushConstantRange& pushConstant)
{
	pushConstants.add(pushConstant);
}

RenderTargetLayout::RenderTargetLayout()
	: inputAttachments()
	, colorAttachments()
	, depthAttachment()
{

}

RenderTargetLayout::RenderTargetLayout(PTexture2D depthAttachment)
	: inputAttachments()
	, colorAttachments()
	, depthAttachment(depthAttachment)
{

}

RenderTargetLayout::RenderTargetLayout(PTexture2D colorAttachment, PTexture2D depthAttachment)
	: inputAttachments()
	, depthAttachment(depthAttachment)
{
	colorAttachments.add(colorAttachment);
}
RenderTargetLayout::RenderTargetLayout(Array<PTexture2D> colorAttachments, PTexture2D depthAttachmet)
	: inputAttachments()
	, colorAttachments(colorAttachments)
	, depthAttachment(depthAttachment)
{

}
RenderTargetLayout::RenderTargetLayout(Array<PTexture2D> inputAttachments, Array<PTexture2D> colorAttachments, PTexture2D depthAttachment)
	: inputAttachments(inputAttachments)
	, colorAttachments(colorAttachments)
	, depthAttachment(depthAttachment)
{

}
