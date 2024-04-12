#include "Descriptor.h"
#include "Graphics.h"

using namespace Seele;
using namespace Seele::Metal;

DescriptorSet::DescriptorSet(PGraphics graphics, PDescriptorPool owner)
  : graphics(graphics)
  , owner(owner)
{
  encoder = graphics->getDevice()->newArgumentEncoder(owner->getArguments());
}
DescriptorSet::~DescriptorSet()
{
}
void DescriptorSet::writeChanges()
{
}

  void DescriptorSet::updateBuffer(uint32_t binding,
                            Gfx::PUniformBuffer uniformBuffer){}
   void DescriptorSet::updateBuffer(uint32_t binding, Gfx::PShaderBuffer uniformBuffer){}
  void DescriptorSet::updateSampler(uint32_t binding, Gfx::PSampler samplerState){}
  void DescriptorSet::updateTexture(uint32_t binding, Gfx::PTexture texture,
                             Gfx::PSampler sampler){}
   void DescriptorSet::updateTextureArray(uint32_t binding,
                                  Array<Gfx::PTexture> texture){}
   bool DescriptorSet::operator<(Gfx::PDescriptorSet other){return this < other.getHandle();}
