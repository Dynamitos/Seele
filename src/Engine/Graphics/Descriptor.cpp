#include "Descriptor.h"

using namespace Seele;
using namespace Seele::Gfx;

DescriptorLayout::DescriptorLayout(const std::string& name) : name(name) {}

DescriptorLayout::~DescriptorLayout() {}

void DescriptorLayout::addDescriptorBinding(DescriptorBinding binding) {
    descriptorBindings.add(binding);
}

ODescriptorSet DescriptorLayout::allocateDescriptorSet() { return pool->allocateDescriptorSet(); }

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

void PipelineLayout::addMapping(std::string mappingName, uint32 index) { parameterMapping[mappingName] = index; }
