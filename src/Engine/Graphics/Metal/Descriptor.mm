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

DescriptorLayout::~DescriptorLayout() { arguments->release(); }

void DescriptorLayout::create() {
    pool = new DescriptorPool(graphics, this);
    hash = CRC::Calculate(descriptorBindings.data(), sizeof(Gfx::DescriptorBinding) * descriptorBindings.size(), CRC::CRC_32());
    MTL::ArgumentDescriptor** objects = new MTL::ArgumentDescriptor*[descriptorBindings.size()];
    uint32 mappingCounter = 0;
    for (uint32 i = 0; i < descriptorBindings.size(); ++i) {
        if (descriptorBindings[i].descriptorType != Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            plainDescriptor = false;
        }
        objects[i] = MTL::ArgumentDescriptor::alloc()->init();
        objects[i]->setIndex(mappingCounter);
        objects[i]->setAccess(MTL::BindingAccessReadOnly);
        objects[i]->setArrayLength(descriptorBindings[i].descriptorCount);
        objects[i]->setDataType(MTL::DataTypeChar);
        objects[i]->setArrayLength(descriptorBindings[i].uniformLength);
    
        variableMapping[descriptorBindings[i].name] = DescriptorMapping{
            .index = mappingCounter,
            .constantSize = descriptorBindings[i].uniformLength,
            .access = descriptorBindings[i].access,
        };
        mappingCounter += descriptorBindings[i].descriptorCount;
    }
    numResources = mappingCounter;
    arguments = NS::Array::array((NS::Object**)objects, descriptorBindings.size());
    delete[] objects;
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
        allocatedSets[setIndex]->reset();
        return PDescriptorSet(allocatedSets[setIndex]);
    }
    allocatedSets.add(new DescriptorSet(graphics, this));
    return PDescriptorSet(allocatedSets.back());
}

void DescriptorPool::reset() {}

DescriptorSet::DescriptorSet(PGraphics graphics, PDescriptorPool owner)
    : Gfx::DescriptorSet(owner->getLayout()), CommandBoundResource(graphics), graphics(graphics), owner(owner) {
    std::cout << "New Descriptor set" << std::endl;
}

DescriptorSet::~DescriptorSet() {
    if(encoder != nullptr) {
        encoder->release();
    }
    if(argumentBuffer != nullptr) {
        argumentBuffer->release();
    }
    std::cout << "destroying descriptor set" << std::endl;
}

void DescriptorSet::reset() {
    uniformWrites.clear();
    bufferWrites.clear();
    textureWrites.clear();
    samplerWrites.clear();
    accelerationWrites.clear();
}

void DescriptorSet::writeChanges() {}

void DescriptorSet::updateConstants(const std::string& name, uint32 offset, void* data) {
    uint32 flattenedIndex = owner->getLayout()->variableMapping[name].index;
    Array<uint8> contents(owner->getLayout()->variableMapping[name].constantSize);
    std::memcpy(contents.data(), (uint8*)data + offset, contents.size());
    uniformWrites.add(UniformWriteInfo{
        .index = flattenedIndex,
        .content = contents,
    });
}

void DescriptorSet::updateBuffer(const std::string& name, uint32 index, Gfx::PShaderBuffer uniformBuffer) {
    uint32 flattenedIndex = owner->getLayout()->variableMapping[name].index + index;
    PShaderBuffer buffer = uniformBuffer.cast<ShaderBuffer>();
    bufferWrites.add(BufferWriteInfo{
        .index = flattenedIndex,
        .buffer = buffer->getHandle(),
        .access = owner->getLayout()->variableMapping[name].access,
    });
}

void DescriptorSet::updateBuffer(const std::string& name, uint32 index, Gfx::PVertexBuffer uniformBuffer) {
    uint32 flattenedIndex = owner->getLayout()->variableMapping[name].index + index;
    PVertexBuffer buffer = uniformBuffer.cast<VertexBuffer>();
    bufferWrites.add(BufferWriteInfo{
        .index = flattenedIndex,
        .buffer = buffer->getHandle(),
        .access = owner->getLayout()->variableMapping[name].access,
    });
}

void DescriptorSet::updateBuffer(const std::string& name, uint32 index, Gfx::PIndexBuffer uniformBuffer) {
    uint32 flattenedIndex = owner->getLayout()->variableMapping[name].index + index;
    PIndexBuffer buffer = uniformBuffer.cast<IndexBuffer>();
    bufferWrites.add(BufferWriteInfo{
        .index = flattenedIndex,
        .buffer = buffer->getHandle(),
        .access = owner->getLayout()->variableMapping[name].access,
    });
}

void DescriptorSet::updateSampler(const std::string& name, uint32 index, Gfx::PSampler samplerState) {
    uint32 flattenedIndex = owner->getLayout()->variableMapping[name].index + index;
    PSampler sampler = samplerState.cast<Sampler>();
    samplerWrites.add(SamplerWriteInfo{
        .index = flattenedIndex,
        .sampler = sampler->getHandle(),
    });
}

void DescriptorSet::updateTexture(const std::string& name, uint32 index, Gfx::PTexture2D texture) {
    uint32 flattenedIndex = owner->getLayout()->variableMapping[name].index + index;
    PTextureBase tex = texture.cast<TextureBase>();
    textureWrites.add(TextureWriteInfo{
        .index = flattenedIndex,
        .texture = tex->getImage(),
        .access = owner->getLayout()->variableMapping[name].access,
    });
}

void DescriptorSet::updateTexture(const std::string& name, uint32 index, Gfx::PTexture3D texture) {
    uint32 flattenedIndex = owner->getLayout()->variableMapping[name].index + index;
    PTextureBase tex = texture.cast<TextureBase>();
    textureWrites.add(TextureWriteInfo{
        .index = flattenedIndex,
        .texture = tex->getImage(),
        .access = owner->getLayout()->variableMapping[name].access,
    });
}

void DescriptorSet::updateTexture(const std::string& name, uint32 index, Gfx::PTextureCube texture) {
    uint32 flattenedIndex = owner->getLayout()->variableMapping[name].index + index;
    PTextureBase tex = texture.cast<TextureBase>();
    textureWrites.add(TextureWriteInfo{
        .index = flattenedIndex,
        .texture = tex->getImage(),
        .access = owner->getLayout()->variableMapping[name].access,
    });
}

void DescriptorSet::updateAccelerationStructure(const std::string& name, uint32 index, Gfx::PTopLevelAS as) { assert(false && "TODO"); }

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
