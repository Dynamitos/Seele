#include "Descriptor.h"

using namespace Seele;
using namespace Seele::Gfx;

DescriptorBinding::DescriptorBinding()
	: binding(0)
	, descriptorType(SE_DESCRIPTOR_TYPE_MAX_ENUM)
	, descriptorCount(0x7fff)
	, shaderStages(SE_SHADER_STAGE_ALL)
{
}

DescriptorBinding::DescriptorBinding(const DescriptorBinding& other)
	: binding(other.binding)
	, descriptorType(other.descriptorType)
	, descriptorCount(other.descriptorCount)
	, shaderStages(other.shaderStages)
{
}

DescriptorBinding& DescriptorBinding::operator=(const DescriptorBinding& other)
{
	if (this != &other)
	{
		binding = other.binding;
		descriptorType = other.descriptorType;
		descriptorCount = other.descriptorCount;
		shaderStages = other.shaderStages;
	}
	return *this;
}

DescriptorPool::DescriptorPool()
{
}

DescriptorPool::~DescriptorPool()
{
}

DescriptorLayout::DescriptorLayout(const std::string& name)
	: setIndex(0)
	, name(name)
{
}

DescriptorLayout::DescriptorLayout(const DescriptorLayout& other)
{
	descriptorBindings.resize(other.descriptorBindings.size());
	for (uint32 i = 0; i < descriptorBindings.size(); ++i)
	{
		descriptorBindings[i] = other.descriptorBindings[i];
	}
}

DescriptorLayout& DescriptorLayout::operator=(const DescriptorLayout& other)
{
	if (this != &other)
	{
		descriptorBindings.resize(other.descriptorBindings.size());
		for (uint32 i = 0; i < descriptorBindings.size(); ++i)
		{
			descriptorBindings[i] = other.descriptorBindings[i];
		}
	}
	return *this;
}

DescriptorLayout::~DescriptorLayout()
{
}

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
	return pool->allocateDescriptorSet();
}

void DescriptorLayout::reset()
{
	pool->reset();
}

PipelineLayout::PipelineLayout()
{
}

PipelineLayout::PipelineLayout(PPipelineLayout baseLayout)
{
	if (baseLayout != nullptr)
	{
		descriptorSetLayouts = baseLayout->descriptorSetLayouts;
		pushConstants = baseLayout->pushConstants;
	}
}

PipelineLayout::~PipelineLayout()
{
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
