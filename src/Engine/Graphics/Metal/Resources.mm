#include "Resources.h"
#include "Enums.h"
#include "Graphics.h"
#include "Metal/MTLSampler.hpp"


using namespace Seele;
using namespace Seele::Metal;

Fence::Fence(PGraphics graphics) : handle(graphics->getDevice()->newFence()) {}

Fence::~Fence() { handle->release(); }

Event::Event(PGraphics graphics) : handle(graphics->getDevice()->newEvent()) {}

Event::~Event() { handle->release(); }

Sampler::Sampler(PGraphics graphics, const SamplerCreateInfo &createInfo) {
  MTL::SamplerDescriptor *desc = MTL::SamplerDescriptor::alloc()->init();
  desc->setBorderColor(cast(createInfo.borderColor));
  desc->setCompareFunction(cast(createInfo.compareOp));
  desc->setLodAverage(createInfo.mipLodBias);
  desc->setLodMaxClamp(createInfo.maxLod);
  desc->setLodMinClamp(createInfo.minLod);
  desc->setMinFilter(cast(createInfo.minFilter));
  desc->setMagFilter(cast(createInfo.magFilter));
  desc->setMipFilter(cast(createInfo.mipmapMode));
  desc->setMaxAnisotropy(createInfo.maxAnisotropy);
  desc->setNormalizedCoordinates(!createInfo.unnormalizedCoordinates);
  desc->setRAddressMode(cast(createInfo.addressModeU));
  desc->setSAddressMode(cast(createInfo.addressModeV));
  desc->setTAddressMode(cast(createInfo.addressModeW));
  desc->setSupportArgumentBuffers(true);
  sampler = graphics->getDevice()->newSamplerState(desc);
  desc->release();
}

Sampler::~Sampler() { sampler->release(); }