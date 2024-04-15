#include "Descriptor.h"

using namespace Seele;
using namespace Seele::Gfx;

DescriptorLayout::DescriptorLayout(const std::string& name) : setIndex(0), name(name) {}

DescriptorLayout::DescriptorLayout(const DescriptorLayout& other) {
  descriptorBindings.resize(other.descriptorBindings.size());
  for (uint32 i = 0; i < descriptorBindings.size(); ++i) {
    descriptorBindings[i] = other.descriptorBindings[i];
  }
}

DescriptorLayout& DescriptorLayout::operator=(const DescriptorLayout& other) {
  if (this != &other) {
    descriptorBindings.resize(other.descriptorBindings.size());
    for (uint32 i = 0; i < descriptorBindings.size(); ++i) {
      descriptorBindings[i] = other.descriptorBindings[i];
    }
  }
  return *this;
}

DescriptorLayout::~DescriptorLayout() {}

void DescriptorLayout::addDescriptorBinding(uint32 bindingIndex, SeDescriptorType type, SeImageViewType textureType, uint32 arrayCount,
                                            SeDescriptorBindingFlags bindingFlags, SeShaderStageFlags shaderStages) {
  if (descriptorBindings.size() <= bindingIndex) {
    descriptorBindings.resize(bindingIndex + 1);
  }
  descriptorBindings[bindingIndex] = DescriptorBinding{
      .binding = bindingIndex,
      .descriptorType = type,
      .textureType = textureType,
      .descriptorCount = arrayCount,
      .bindingFlags = bindingFlags,
      .shaderStages = shaderStages,
  };
}

PDescriptorSet DescriptorLayout::allocateDescriptorSet() { return pool->allocateDescriptorSet(); }

void DescriptorLayout::reset() { pool->reset(); }

DescriptorPool::DescriptorPool() {}

DescriptorPool::~DescriptorPool() {}

DescriptorSet::DescriptorSet(PDescriptorLayout layout) : layout(layout) {}

DescriptorSet::~DescriptorSet() {}

PipelineLayout::PipelineLayout() {}

PipelineLayout::PipelineLayout(PPipelineLayout baseLayout) {
  if (baseLayout != nullptr) {
    descriptorSetLayouts = baseLayout->descriptorSetLayouts;
    pushConstants = baseLayout->pushConstants;
  }
}

PipelineLayout::~PipelineLayout() {}

void PipelineLayout::addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout) {
  if (descriptorSetLayouts.size() <= setIndex) {
    descriptorSetLayouts.resize(setIndex + 1);
  }
  descriptorSetLayouts[setIndex] = layout;
  layout->setIndex = setIndex;
}

void PipelineLayout::addPushConstants(const SePushConstantRange& pushConstant) { pushConstants.add(pushConstant); }
