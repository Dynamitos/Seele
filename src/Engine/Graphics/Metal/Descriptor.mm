#include "Descriptor.h"
#include "Buffer.h"
#include "Enums.h"
#include "Graphics/Initializer.h"
#include "Texture.h"

using namespace Seele;
using namespace Seele::Metal;

DescriptorLayout::DescriptorLayout(PGraphics graphics, const std::string& name)
    : Gfx::DescriptorLayout(name), graphics(graphics) {}

DescriptorLayout::~DescriptorLayout() {}

void DescriptorLayout::create() {
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
      dataType = MTL::DataTypeStruct;
      break;
    case Gfx::SE_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
      dataType = MTL::DataTypePrimitiveAccelerationStructure;
      break;
    default:
      throw std::logic_error("Nooo");
    }
    desc->setDataType(dataType);
    descriptors.add(desc);
  }
  arguments = NS::Array::array(descriptors.data(), descriptors.size());
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
  return PDescriptorSet(allocatedSets.back());
}

void DescriptorPool::reset() {
  for (auto& set : allocatedSets) {
    set->free();
  }
}

DescriptorSet::DescriptorSet(PGraphics graphics, PDescriptorPool owner)
    : Gfx::DescriptorSet(owner->getLayout()), graphics(graphics), owner(owner), bindCount(0), currentlyInUse(false) {
  encoder = graphics->getDevice()->newArgumentEncoder(owner->getArguments());
  buffer = graphics->getDevice()->newBuffer(encoder->encodedLength(), MTL::ResourceOptionCPUCacheModeDefault);
  encoder->setArgumentBuffer(buffer, 0);
}

DescriptorSet::~DescriptorSet() {}

void DescriptorSet::writeChanges() {}

void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer) {
  PUniformBuffer metalBuffer = uniformBuffer.cast<UniformBuffer>();
  encoder->setBuffer(metalBuffer->getHandle(), 0, binding);
}
void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PShaderBuffer uniformBuffer) {
  PShaderBuffer metalBuffer = uniformBuffer.cast<ShaderBuffer>();
  encoder->setBuffer(metalBuffer->getHandle(), 0, binding);
}
void DescriptorSet::updateSampler(uint32_t binding, Gfx::PSampler samplerState) {
  PSampler sampler = samplerState.cast<Sampler>();
  encoder->setSamplerState(sampler->getHandle(), binding);
}
void DescriptorSet::updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSampler samplerState) {
  PTextureBase base = texture.cast<TextureBase>();
  PSampler sampler = samplerState.cast<Sampler>();
  encoder->setTexture(base->getTexture(), binding);
  encoder->setSamplerState(sampler->getHandle(), binding);
}

void DescriptorSet::updateTextureArray(uint32_t binding, Array<Gfx::PTexture> array) {
  for (auto& t : array) {
    PTextureBase metalTexture = t.cast<TextureBase>();
    encoder->setTexture(metalTexture->getTexture(), binding++);
  }
}

bool DescriptorSet::operator<(Gfx::PDescriptorSet other) { return this < other.getHandle(); }

PipelineLayout::PipelineLayout(PGraphics graphics, Gfx::PPipelineLayout baseLayout)
      : Gfx::PipelineLayout(baseLayout), graphics(graphics) {}

PipelineLayout::~PipelineLayout(){}

void PipelineLayout::create() 
{
  for(auto& set : descriptorSetLayouts)
  {
    uint32 setHash = set->getHash();
    layoutHash = CRC::Calculate(&setHash, sizeof(uint32), CRC::CRC_32(), layoutHash);
  }
}