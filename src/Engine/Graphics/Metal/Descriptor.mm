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
    : Gfx::DescriptorLayout(name), graphics(graphics) {}

DescriptorLayout::~DescriptorLayout() {}

void DescriptorLayout::create() {
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
  boundResources.resize(owner->getLayout()->getBindings().size());
  buffer = graphics->getDevice()->newBuffer(std::max<size_t>(8, sizeof(uint64_t) * 3 * owner->getLayout()->getBindings().size()), MTL::ResourceStorageModeShared);
    argumentBuffer = (uint64*)buffer->contents();
    buffer->setLabel(NS::String::string(owner->getLayout()->getName().c_str(), NS::ASCIIStringEncoding));
}

DescriptorSet::~DescriptorSet() {buffer->release();}

void DescriptorSet::writeChanges() {}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer) {
  PUniformBuffer metalBuffer = uniformBuffer.cast<UniformBuffer>();
    uint64 offset = binding * 3;
    argumentBuffer[offset + 0] = metalBuffer->getHandle()->gpuAddress();
    argumentBuffer[offset + 1] = 0;
    argumentBuffer[offset + 2] = (uint32)metalBuffer->getSize(); // TODO: buffer texture view, typed??
    boundResources[binding] = metalBuffer->getHandle();
}
void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PShaderBuffer uniformBuffer) {
  PShaderBuffer metalBuffer = uniformBuffer.cast<ShaderBuffer>();
    uint64 offset = binding * 3;
    argumentBuffer[offset + 0] = metalBuffer->getHandle()->gpuAddress();
    argumentBuffer[offset + 1] = 0;
    argumentBuffer[offset + 2] = (uint32)metalBuffer->getSize(); // TODO: buffer texture view, typed??
    boundResources[binding] = metalBuffer->getHandle();
}
void DescriptorSet::updateSampler(uint32_t binding, Gfx::PSampler samplerState) {
  PSampler sampler = samplerState.cast<Sampler>();
    MTL::ResourceID resourceId =sampler->getHandle()->gpuResourceID();
    uint64 offset = binding * 3;
    argumentBuffer[offset + 0] = 0;
    argumentBuffer[offset + 1] = *(uint64*)&resourceId;
    argumentBuffer[offset + 2] = 0; // LOD bias
}
void DescriptorSet::updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSampler samplerState) {
  PTextureBase base = texture.cast<TextureBase>();
    if(layout->getBindings()[binding].access == Gfx::SE_DESCRIPTOR_ACCESS_READ_ONLY_BIT)
    {
        MTL::ResourceID resourceId =base->getTexture()->gpuResourceID();
        uint64 offset = binding * 3;
        argumentBuffer[offset + 0] = 0;
        argumentBuffer[offset + 1] = *(uint64*)&resourceId;
        argumentBuffer[offset + 2] = 0; // min LOD clamp
    }else{
        uint64 offset = binding * 3;
        argumentBuffer[offset + 0] = base->getTexture()->buffer()->gpuAddress();
        argumentBuffer[offset + 1] = 0;
        argumentBuffer[offset + 2] = (uint32)base->getTexture()->buffer()->length(); // TODO: buffer texture view, typed??
    }
    boundResources[binding] = base->getTexture();
}

void DescriptorSet::updateTextureArray(uint32_t binding, Array<Gfx::PTexture> array) {
  for (auto& t : array) {
      updateTexture(binding++, t);
  }
}

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
