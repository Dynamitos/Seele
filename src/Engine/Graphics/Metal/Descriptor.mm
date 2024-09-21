#include "Descriptor.h"
#include "Buffer.h"
#include "Enums.h"
#include "Foundation/NSArray.hpp"
#include "Foundation/NSObject.hpp"
#include "Foundation/NSString.hpp"
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
#include <fmt/format.h>

using namespace Seele;
using namespace Seele::Metal;

DescriptorLayout::DescriptorLayout(PGraphics graphics, const std::string& name) : Gfx::DescriptorLayout(name), graphics(graphics) {}

DescriptorLayout::~DescriptorLayout() {}

void DescriptorLayout::create() {
    pool = new DescriptorPool(graphics, this);
    hash = CRC::Calculate(descriptorBindings.data(), sizeof(Gfx::DescriptorBinding) * descriptorBindings.size(), CRC::CRC_32());
    MTL::ArgumentDescriptor** objects = new MTL::ArgumentDescriptor*[descriptorBindings.size()];
    for (uint32 i = 0; i < descriptorBindings.size(); ++i) {
        objects[i] = MTL::ArgumentDescriptor::alloc()->init();
        objects[i]->setIndex(i);
        objects[i]->setAccess(cast(descriptorBindings[i].access));
        objects[i]->setArrayLength(descriptorBindings[i].descriptorCount);
        MTL::DataType dataType;
        switch (descriptorBindings[i].descriptorType) {
        case Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case Gfx::SE_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        case Gfx::SE_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            dataType = MTL::DataTypeTexture;
            break;
        case Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case Gfx::SE_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            dataType = MTL::DataTypePointer;
            break;
        case Gfx::SE_DESCRIPTOR_TYPE_SAMPLER:
            dataType = MTL::DataTypeSampler;
            break;
        case Gfx::SE_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            dataType = MTL::DataTypeInstanceAccelerationStructure;
            break;
        default:
            throw new std::logic_error("unknown descriptor type");
        }
        objects[i]->setDataType(dataType);
        MTL::TextureType textureType;
        switch (descriptorBindings[i].textureType) {
        case Gfx::SE_IMAGE_VIEW_TYPE_1D:
            textureType = MTL::TextureType1D;
            break;
        case Gfx::SE_IMAGE_VIEW_TYPE_2D:
            textureType = MTL::TextureType2D;
            break;
        case Gfx::SE_IMAGE_VIEW_TYPE_3D:
            textureType = MTL::TextureType3D;
            break;
        case Gfx::SE_IMAGE_VIEW_TYPE_CUBE:
            textureType = MTL::TextureTypeCube;
            break;
        case Gfx::SE_IMAGE_VIEW_TYPE_1D_ARRAY:
            textureType = MTL::TextureType1DArray;
            break;
        case Gfx::SE_IMAGE_VIEW_TYPE_2D_ARRAY:
            textureType = MTL::TextureType2DArray;
            break;
        case Gfx::SE_IMAGE_VIEW_TYPE_CUBE_ARRAY:
            textureType = MTL::TextureTypeCubeArray;
            break;
        }
        objects[i]->setTextureType(textureType);
    }
    arguments = NS::Array::array((NS::Object**)objects, descriptorBindings.size());
    std::cout << "Layout " << name << std::endl;
    std::cout << arguments->debugDescription()->cString(NS::ASCIIStringEncoding) << std::endl;
}

MTL::ArgumentEncoder* DescriptorLayout::createEncoder() {
    MTL::AutoreleasedArgument arg;
    auto enc =  function->newArgumentEncoder(index, &arg);
    std::cout << arg->debugDescription()->cString(NS::ASCIIStringEncoding) << std::endl;
    return enc;
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
    encoder = owner->getLayout()->createEncoder();
    argumentBuffer = graphics->getDevice()->newBuffer(encoder->encodedLength(), 0);
    argumentBuffer->setLabel(NS::String::string(fmt::format("{0}ArgumentBuffer", owner->getLayout()->getName()).c_str(), NS::ASCIIStringEncoding));
    encoder->setArgumentBuffer(argumentBuffer, 0);
    boundResources.resize(owner->getLayout()->getBindings().size());
}

DescriptorSet::~DescriptorSet() {}

void DescriptorSet::writeChanges() {}

void DescriptorSet::updateBuffer(uint32 binding, Gfx::PUniformBuffer uniformBuffer) {
    PUniformBuffer buffer = uniformBuffer.cast<UniformBuffer>();
    encoder->setBuffer(buffer->getHandle(), 0, binding);
    boundResources[binding] = buffer->getHandle();
}

void DescriptorSet::updateBuffer(uint32 binding, Gfx::PShaderBuffer uniformBuffer) {
    PShaderBuffer buffer = uniformBuffer.cast<ShaderBuffer>();
    encoder->setBuffer(buffer->getHandle(), 0, binding);
    boundResources[binding] = buffer->getHandle();
}

void DescriptorSet::updateBuffer(uint32 binding, Gfx::PIndexBuffer uniformBuffer) {
    PIndexBuffer buffer = uniformBuffer.cast<IndexBuffer>();
    encoder->setBuffer(buffer->getHandle(), 0, binding);
    boundResources[binding] = buffer->getHandle();
}

void DescriptorSet::updateSampler(uint32 binding, Gfx::PSampler samplerState) {
    PSampler sampler = samplerState.cast<Sampler>();
    encoder->setSamplerState(sampler->getHandle(), binding);
    boundResources[binding] = nullptr; // Samplers are not resources??????
}

void DescriptorSet::updateTexture(uint32 binding, Gfx::PTexture texture, Gfx::PSampler sampler) {
    PTextureBase base = texture.cast<TextureBase>();
    encoder->setTexture(base->getHandle()->texture, binding);
    boundResources[binding] = base->getHandle()->texture;
}

void DescriptorSet::updateTextureArray(uint32 binding, Array<Gfx::PTexture2D> texture) { assert(false && "TODO"); }

void DescriptorSet::updateSamplerArray(uint32 binding, Array<Gfx::PSampler> samplers) { assert(false && "TODO"); }

void DescriptorSet::updateAccelerationStructure(uint32 binding, Gfx::PTopLevelAS as) { assert(false && "TODO"); }

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
