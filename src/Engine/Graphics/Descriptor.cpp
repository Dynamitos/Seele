#include "Descriptor.h"

using namespace Seele;
using namespace Seele::Gfx;

void DescriptorLayout::addDescriptorBinding(uint32 bindingIndex, SeDescriptorType type, uint32 arrayCount, SeDescriptorBindingFlags bindingFlags, SeShaderStageFlags shaderStages)
{
	if (descriptorBindings.size() <= bindingIndex)
	{
		descriptorBindings.resize(bindingIndex + 1);
	}
	DescriptorBinding& binding = descriptorBindings[bindingIndex];
	binding.binding = bindingIndex;
	binding.descriptorType = type;
	binding.descriptorCount = arrayCount;
	binding.bindingFlags = bindingFlags;
	binding.shaderStages = shaderStages;
}

PDescriptorSet DescriptorLayout::allocateDescriptorSet()
{
	std::scoped_lock lock(allocatorLock);
	PDescriptorSet result;
	allocator->allocateDescriptorSet(result);
	return result;
}

void DescriptorLayout::reset()
{
	std::scoped_lock lock(allocatorLock);
	allocator->reset();
}

void PipelineLayout::addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout)
{
	if (descriptorSetLayouts.size() <= setIndex)
	{
		descriptorSetLayouts.resize(setIndex + 1);
	}
	descriptorSetLayouts[setIndex] = layout;
	layout->setIndex = setIndex;
}

void PipelineLayout::addPushConstants(const SePushConstantRange& pushConstant)
{
	pushConstants.add(pushConstant);
}
