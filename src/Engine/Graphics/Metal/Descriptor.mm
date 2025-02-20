#include "Descriptor.h"
#include "Buffer.h"
#include "Enums.h"
#include "Foundation/NSArray.hpp"
#include "Foundation/NSObject.hpp"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
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
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>
#include <iostream>
#include <stdexcept>

using namespace Seele;
using namespace Seele::Metal;

DescriptorLayout::DescriptorLayout(PGraphics graphics, const std::string& name) : Gfx::DescriptorLayout(name), graphics(graphics) {}

DescriptorLayout::~DescriptorLayout() {}

void DescriptorLayout::create() {
    pool = new DescriptorPool(graphics, this);
    hash = CRC::Calculate(descriptorBindings.data(), sizeof(Gfx::DescriptorBinding) * descriptorBindings.size(), CRC::CRC_32());
    MTL::ArgumentDescriptor** objects = new MTL::ArgumentDescriptor*[descriptorBindings.size()];
    flattenedBindingCount = 0;
    for (uint32 i = 0; i < descriptorBindings.size(); ++i) {
        if(descriptorBindings[i].descriptorType != Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            plainDescriptor = false;
        } else {
            objects[i] = MTL::ArgumentDescriptor::alloc()->init();
            objects[i]->setIndex(flattenedBindingCount);
            objects[i]->setAccess(cast(descriptorBindings[i].access));
            objects[i]->setArrayLength(descriptorBindings[i].descriptorCount);
            objects[i]->setDataType(MTL::DataTypeChar);
            objects[i]->setArrayLength(descriptorBindings[i].uniformLength);
        }
        for(uint32 j = 0; j < descriptorBindings[i].descriptorCount; ++j) {
            flattenMap[flattenIndex(i, j)] = flattenedBindingCount++;
        }
    }
    arguments = NS::Array::array((NS::Object**)objects, descriptorBindings.size());
}

MTL::ArgumentEncoder* DescriptorLayout::createEncoder() { return graphics->getDevice()->newArgumentEncoder(arguments); }

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
    boundResources.resize(owner->getLayout()->getTotalBindingCount());
}

DescriptorSet::~DescriptorSet() {}

void DescriptorSet::writeChanges() {}

void DescriptorSet::updateBuffer(uint32 binding, uint32 index, Gfx::PUniformBuffer uniformBuffer) {
    uint32 flattenedIndex = owner->getLayout()->getFlattenedIndex(binding, index);
    PUniformBuffer buffer = uniformBuffer.cast<UniformBuffer>();
    boundResources[flattenedIndex] = nullptr;
    uniformWrites.add(UniformWriteInfo{
        .index = flattenedIndex,
        .content = buffer->getContents(),
    });
}

void DescriptorSet::updateBuffer(uint32 binding, uint32 index, Gfx::PShaderBuffer uniformBuffer) {
    uint32 flattenedIndex = owner->getLayout()->getFlattenedIndex(binding, index);
    PShaderBuffer buffer = uniformBuffer.cast<ShaderBuffer>();
    boundResources[flattenedIndex] = buffer->getHandle();
    bufferWrites.add(BufferWriteInfo{
        .index = flattenedIndex,
        .buffer = buffer->getHandle(),
    });
}

void DescriptorSet::updateBuffer(uint32 binding, uint32 index, Gfx::PVertexBuffer uniformBuffer) {
    uint32 flattenedIndex = owner->getLayout()->getFlattenedIndex(binding, index);
    PVertexBuffer buffer = uniformBuffer.cast<VertexBuffer>();
    boundResources[flattenedIndex] = buffer->getHandle();
    bufferWrites.add(BufferWriteInfo{
        .index = flattenedIndex,
        .buffer = buffer->getHandle(),
    });
}

void DescriptorSet::updateBuffer(uint32 binding, uint32 index, Gfx::PIndexBuffer uniformBuffer) {
    uint32 flattenedIndex = owner->getLayout()->getFlattenedIndex(binding, index);
    PIndexBuffer buffer = uniformBuffer.cast<IndexBuffer>();
    boundResources[flattenedIndex] = buffer->getHandle();
    bufferWrites.add(BufferWriteInfo{
        .index = flattenedIndex,
        .buffer = buffer->getHandle(),
    });
}

void DescriptorSet::updateSampler(uint32 binding, uint32 index, Gfx::PSampler samplerState) {
    uint32 flattenedIndex = owner->getLayout()->getFlattenedIndex(binding, index);
    PSampler sampler = samplerState.cast<Sampler>();
    boundResources[flattenedIndex] = nullptr;
    samplerWrites.add(SamplerWriteInfo{
        .index = flattenedIndex,
        .sampler = sampler->getHandle(),
    });
}

void DescriptorSet::updateTexture(uint32 binding, uint32 index, Gfx::PTexture2D texture) {
    uint32 flattenedIndex = owner->getLayout()->getFlattenedIndex(binding, index);
    PTextureBase tex = texture.cast<TextureBase>();
    boundResources[flattenedIndex] = tex->getImage();
    textureWrites.add(TextureWriteInfo{
        .index = flattenedIndex,
        .texture = tex->getImage(),
    });
}

void DescriptorSet::updateTexture(uint32 binding, uint32 index, Gfx::PTexture3D texture) {
    uint32 flattenedIndex = owner->getLayout()->getFlattenedIndex(binding, index);
    PTextureBase tex = texture.cast<TextureBase>();
    boundResources[flattenedIndex] = tex->getImage();
    textureWrites.add(TextureWriteInfo{
        .index = flattenedIndex,
        .texture = tex->getImage(),
    });
}

void DescriptorSet::updateTexture(uint32 binding, uint32 index, Gfx::PTextureCube texture) {
    uint32 flattenedIndex = owner->getLayout()->getFlattenedIndex(binding, index);
    PTextureBase tex = texture.cast<TextureBase>();
    boundResources[flattenedIndex] = tex->getImage();
    textureWrites.add(TextureWriteInfo{
        .index = flattenedIndex,
        .texture = tex->getImage(),
    });
}

void DescriptorSet::updateAccelerationStructure(uint32 binding, uint32 index, Gfx::PTopLevelAS as) { assert(false && "TODO"); }

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
