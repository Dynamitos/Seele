#include "Descriptor.h"
#include "Buffer.h"
#include "Enums.h"
#include "Foundation/NSArray.hpp"
#include "Foundation/NSObject.hpp"
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Resources.h"
#include "Graphics/Metal/Shader.h"
#include "Metal/MTLArgument.hpp"
#include "Metal/MTLArgumentEncoder.hpp"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLResource.hpp"
#include "Metal/MTLStageInputOutputDescriptor.hpp"
#include "Metal/MTLTexture.hpp"
#include "Texture.h"
#include <CRC.h>
#include <Foundation/Foundation.h>
#include <iostream>

using namespace Seele;
using namespace Seele::Metal;

DescriptorLayout::DescriptorLayout(PGraphics graphics, const std::string& name) : Gfx::DescriptorLayout(name), graphics(graphics) {}

DescriptorLayout::~DescriptorLayout() {}

void DescriptorLayout::create() {
    pool = new DescriptorPool(graphics, this);
    hash = CRC::Calculate(descriptorBindings.data(), sizeof(Gfx::DescriptorBinding) * descriptorBindings.size(), CRC::CRC_32());
    MTL::ArgumentDescriptor** objects = new MTL::ArgumentDescriptor*[descriptorBindings.size()];
    for(uint32 i = 0; i < descriptorBindings.size(); ++i) {
        objects[i] = MTL::ArgumentDescriptor::alloc()->init();
        objects[i]->setIndex(i);
        objects[i]->setAccess(cast(descriptorBindings[i].access));
        objects[i]->setArrayLength(descriptorBindings[i].descriptorCount);
        objects[i]->setDataType(MTL::DataTypeStruct);
    }
    arguments = NS::Array::array((NS::Object**)objects, descriptorBindings.size());
}

MTL::ArgumentEncoder* DescriptorLayout::createEncoder() {
    return graphics->getDevice()->newArgumentEncoder(arguments);
}

DescriptorPool::DescriptorPool(PGraphics graphics, PDescriptorLayout layout) : graphics(graphics), layout(layout) {}

DescriptorPool::~DescriptorPool() {}

Gfx::PDescriptorSet DescriptorPool::allocateDescriptorSet() {
    for (uint32 setIndex = 0; setIndex < allocatedSets.size(); ++setIndex) {
        if (allocatedSets[setIndex]->isCurrentlyBound()) {
            // Currently in use, skip
            continue;
        }

        // Found set, stop searching
        return PDescriptorSet(allocatedSets[setIndex]);
    }
    allocatedSets.add(new DescriptorSet(graphics, this));
    return PDescriptorSet(allocatedSets.back());
}

void DescriptorPool::reset() {}

DescriptorSet::DescriptorSet(PGraphics graphics, PDescriptorPool owner)
    : Gfx::DescriptorSet(owner->getLayout()), CommandBoundResource(graphics), graphics(graphics), owner(owner) {
    boundResources.resize(owner->getLayout()->getBindings().size());
    for (uint32 i = 0; i < boundResources.size(); ++i) {
        boundResources[i].resize(owner->getLayout()->getBindings()[i].descriptorCount);
    }
}

DescriptorSet::~DescriptorSet() {}

void DescriptorSet::writeChanges() {}

void DescriptorSet::updateBuffer(uint32 binding, Gfx::PUniformBuffer uniformBuffer) {
    boundResources[binding][0] = uniformBuffer.cast<UniformBuffer>()->getHandle();
}

void DescriptorSet::updateBuffer(uint32 binding, Gfx::PShaderBuffer uniformBuffer) {
    boundResources[binding][0] = uniformBuffer.cast<ShaderBuffer>()->getHandle();
}

void DescriptorSet::updateBuffer(uint32 binding, Gfx::PIndexBuffer uniformBuffer) {
    boundResources[binding][0] = uniformBuffer.cast<IndexBuffer>()->getHandle();
}

void DescriptorSet::updateBuffer(uint32 binding, uint32 index, Gfx::PShaderBuffer uniformBuffer) {
    boundResources[binding][0] = uniformBuffer.cast<ShaderBuffer>()->getHandle();
}

void DescriptorSet::updateSampler(uint32 binding, Gfx::PSampler samplerState) {
    boundResources[binding][0] = nullptr; // Samplers are not resources??????
}

void DescriptorSet::updateSampler(uint32 binding, uint32 dstArrayIndex, Gfx::PSampler samplerState) {
    boundResources[binding][dstArrayIndex] = nullptr;
}

void DescriptorSet::updateTexture(uint32 binding, Gfx::PTexture texture, Gfx::PSampler sampler) {}

void DescriptorSet::updateTexture(uint32 binding, uint32 dstArrayIndex, Gfx::PTexture texture) {}

void DescriptorSet::updateTextureArray(uint32 binding, Array<Gfx::PTexture2D> texture) {}

void DescriptorSet::updateSamplerArray(uint32 binding, Array<Gfx::PSampler> samplers) {}

void DescriptorSet::updateAccelerationStructure(uint32 binding, Gfx::PTopLevelAS as) {}

PipelineLayout::PipelineLayout(PGraphics graphics, const std::string& name, Gfx::PPipelineLayout baseLayout)
    : Gfx::PipelineLayout(name, baseLayout), graphics(graphics) {}

PipelineLayout::~PipelineLayout() {}

void PipelineLayout::create() {
    for (auto& [_, set] : descriptorSetLayouts) {
        assert(set->getHash() != 0);
        uint32 setHash = set->getHash();
        layoutHash = CRC::Calculate(&setHash, sizeof(uint32), CRC::CRC_32(), layoutHash);
    }
}
