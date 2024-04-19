#include "Descriptor.h"
#include "Buffer.h"
#include "Enums.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "Metal/MTLArgument.hpp"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLResource.hpp"
#include "Metal/MTLTexture.hpp"
#include "Texture.h"
#include <Foundation/Foundation.h>
#include <iostream>

using namespace Seele;
using namespace Seele::Metal;

DescriptorLayout::DescriptorLayout(PGraphics graphics, const std::string& name)
    : Gfx::DescriptorLayout(name), graphics(graphics), arguments(nullptr) {}

DescriptorLayout::~DescriptorLayout() {}

void DescriptorLayout::create() {
  if (arguments != nullptr) {
    return;
  }
  Array<NS::Object*> descriptors;
  for (size_t i = 0; i < descriptorBindings.size(); ++i) {
    const auto& binding = descriptorBindings[i];
    auto desc = MTL::ArgumentDescriptor::alloc()->init();
    desc->setAccess(cast(binding.access));
    desc->setArrayLength(binding.descriptorCount);
    MTL::DataType dataType;
    switch (binding.descriptorType) {
    case Gfx::SE_DESCRIPTOR_TYPE_SAMPLER:
    case Gfx::SE_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    case Gfx::SE_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      dataType = MTL::DataTypeTexture;
      break;
    case Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
    case Gfx::SE_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
      dataType = MTL::DataTypePointer;
      break;
    case Gfx::SE_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
      dataType = MTL::DataTypePrimitiveAccelerationStructure;
      break;
    default:
      throw std::logic_error("Nooo");
    }
    desc->setDataType(dataType);
    desc->setIndex(i);
    if (dataType == MTL::DataTypeTexture) {
      switch (binding.textureType) {
      case Gfx::SE_IMAGE_VIEW_TYPE_1D:
        desc->setTextureType(MTL::TextureType1D);
        break;
      case Gfx::SE_IMAGE_VIEW_TYPE_2D:
        desc->setTextureType(MTL::TextureType2D);
        break;
      case Gfx::SE_IMAGE_VIEW_TYPE_3D:
        desc->setTextureType(MTL::TextureType3D);
        break;
      case Gfx::SE_IMAGE_VIEW_TYPE_CUBE:
        desc->setTextureType(MTL::TextureTypeCube);
        break;
      case Gfx::SE_IMAGE_VIEW_TYPE_1D_ARRAY:
        desc->setTextureType(MTL::TextureType1DArray);
        break;
      case Gfx::SE_IMAGE_VIEW_TYPE_2D_ARRAY:
        desc->setTextureType(MTL::TextureType2DArray);
        break;
      case Gfx::SE_IMAGE_VIEW_TYPE_CUBE_ARRAY:
        desc->setTextureType(MTL::TextureTypeCubeArray);
        break;
      }
    }
    descriptors.add(desc);
  }
  arguments = NS::Array::array(descriptors.data(), descriptors.size());
  pool = new DescriptorPool(graphics, this);
  hash = CRC::Calculate(descriptorBindings.data(), sizeof(Gfx::DescriptorBinding) * descriptorBindings.size(),
                        CRC::CRC_32());
}

DescriptorPool::DescriptorPool(PGraphics graphics, PDescriptorLayout layout) : graphics(graphics), layout(layout) {}

DescriptorPool::~DescriptorPool() {}

Gfx::PDescriptorSet DescriptorPool::allocateDescriptorSet() {
  for (uint32 setIndex = 0; setIndex < allocatedSets.size(); ++setIndex) {
    if (allocatedSets[setIndex]->isCurrentlyBound() || allocatedSets[setIndex]->isCurrentlyInUse()) {
      // Currently in use, skip
      continue;
    }
    allocatedSets[setIndex]->allocate();

    // Found set, stop searching
    return PDescriptorSet(allocatedSets[setIndex]);
  }
  allocatedSets.add(new DescriptorSet(graphics, this));
  allocatedSets.back()->allocate();
  return PDescriptorSet(allocatedSets.back());
}

void DescriptorPool::reset() {
  for (auto& set : allocatedSets) {
    set->free();
  }
}

DescriptorSet::DescriptorSet(PGraphics graphics, PDescriptorPool owner)
    : Gfx::DescriptorSet(owner->getLayout()), graphics(graphics), owner(owner), bindCount(0), currentlyInUse(false) {
  if (owner->getArguments()->count() > 0) {
      boundResources.resize(owner->getArguments()->count());
      encoder = graphics->getDevice()->newArgumentEncoder(owner->getArguments());
      buffer = graphics->getDevice()->newBuffer(encoder->encodedLength(), MTL::ResourceOptionCPUCacheModeDefault);
      encoder->setArgumentBuffer(buffer, 0);
  } else
  {
      buffer = graphics->getDevice()->newBuffer(8, MTL::ResourceOptionCPUCacheModeDefault);
  }
}

DescriptorSet::~DescriptorSet() {}

void DescriptorSet::writeChanges() {}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer) {
  PUniformBuffer metalBuffer = uniformBuffer.cast<UniformBuffer>();
  encoder->setBuffer(metalBuffer->getHandle(), 0, binding);
    boundResources[binding] = metalBuffer->getHandle();
}
void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PShaderBuffer uniformBuffer) {
  PShaderBuffer metalBuffer = uniformBuffer.cast<ShaderBuffer>();
  encoder->setBuffer(metalBuffer->getHandle(), 0, binding);
    boundResources[binding] = metalBuffer->getHandle();
}
void DescriptorSet::updateSampler(uint32_t binding, Gfx::PSampler samplerState) {
  PSampler sampler = samplerState.cast<Sampler>();
  encoder->setSamplerState(sampler->getHandle(), binding);
}
void DescriptorSet::updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSampler samplerState) {
  PTextureBase base = texture.cast<TextureBase>();
    if(layout->getBindings()[binding].access == Gfx::SE_DESCRIPTOR_ACCESS_READ_ONLY_BIT)
    {
        encoder->setTexture(base->getTexture(), binding);
        
    }else{
        encoder->setBuffer(base->getTexture()->buffer(), 0, binding);
    }
    boundResources[binding] = base->getTexture();
}

void DescriptorSet::updateTextureArray(uint32_t binding, Array<Gfx::PTexture> array) {
  for (auto& t : array) {
    PTextureBase metalTexture = t.cast<TextureBase>();
    encoder->setTexture(metalTexture->getTexture(), binding);
      boundResources[binding++] = metalTexture->getTexture();
  }
}

PipelineLayout::PipelineLayout(PGraphics graphics, Gfx::PPipelineLayout baseLayout)
    : Gfx::PipelineLayout(baseLayout), graphics(graphics) {}

PipelineLayout::~PipelineLayout() {}

void PipelineLayout::create() {
  for (auto& [_, set] : descriptorSetLayouts) {
    set->create();
    uint32 setHash = set->getHash();
    layoutHash = CRC::Calculate(&setHash, sizeof(uint32), CRC::CRC_32(), layoutHash);
  }
}
