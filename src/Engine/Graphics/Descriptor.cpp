#include "Descriptor.h"

using namespace Seele;
using namespace Seele::Gfx;

DescriptorLayout::DescriptorLayout(const std::string& name) : name(name) {}

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

void DescriptorLayout::addDescriptorBinding(DescriptorBinding binding) {
    if (descriptorBindings.size() <= binding.binding) {
        descriptorBindings.resize(binding.binding + 1);
    }
    descriptorBindings[binding.binding] = binding;
}

PDescriptorSet DescriptorLayout::allocateDescriptorSet() { return pool->allocateDescriptorSet(); }

void DescriptorLayout::reset() { pool->reset(); }

DescriptorPool::DescriptorPool() {}

DescriptorPool::~DescriptorPool() {}

DescriptorSet::DescriptorSet(PDescriptorLayout layout) : layout(layout) {}

DescriptorSet::~DescriptorSet() {}

PipelineLayout::PipelineLayout(const std::string& name) : name(name) {}

PipelineLayout::PipelineLayout(const std::string& name, PPipelineLayout baseLayout) : name(name) {
    if (baseLayout != nullptr) {
        descriptorSetLayouts = baseLayout->descriptorSetLayouts;
        pushConstants = baseLayout->pushConstants;
    }
}

PipelineLayout::~PipelineLayout() {}

void PipelineLayout::addDescriptorLayout(PDescriptorLayout layout) { descriptorSetLayouts[layout->getName()] = layout; }

void PipelineLayout::addPushConstants(const SePushConstantRange& pushConstant) { pushConstants.add(pushConstant); }

void PipelineLayout::addMapping(std::string name, uint32 index) {
    parameterMapping[name] = index;
}
