#pragma once
#include "Foundation/NSArray.hpp"
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "Metal/MTLArgumentEncoder.hpp"
#include "MinimalEngine.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
DECLARE_REF(DescriptorPool)
class DescriptorLayout : public Gfx::DescriptorLayout {
public:
  DescriptorLayout(PGraphics graphics, const std::string &name);
  virtual ~DescriptorLayout();
  virtual void create() override;
  NS::Array* getArguments() const
  {
    return arguments;
  }
private:
  PGraphics graphics;
  NS::Array* arguments;
};
class PipelineLayout : public Gfx::PipelineLayout {
public:
  PipelineLayout(PGraphics graphics, Gfx::PPipelineLayout baseLayout)
      : Gfx::PipelineLayout(baseLayout), graphics(graphics) {}

private:
  PGraphics graphics;
};
DEFINE_REF(PipelineLayout)

class DescriptorSet : public Gfx::DescriptorSet {
public:
  DescriptorSet(PGraphics graphics, PDescriptorPool owner);
  virtual ~DescriptorSet();
  virtual void writeChanges();
  virtual void updateBuffer(uint32_t binding,
                            Gfx::PUniformBuffer uniformBuffer);
  virtual void updateBuffer(uint32_t binding, Gfx::PShaderBuffer uniformBuffer);
  virtual void updateSampler(uint32_t binding, Gfx::PSampler samplerState);
  virtual void updateTexture(uint32_t binding, Gfx::PTexture texture,
                             Gfx::PSampler sampler = nullptr);
  virtual void updateTextureArray(uint32_t binding,
                                  Array<Gfx::PTexture> texture);
  virtual bool operator<(Gfx::PDescriptorSet other);

private:
  PGraphics graphics;
  PDescriptorPool owner;
  MTL::Buffer* buffer;
  MTL::ArgumentEncoder* encoder;
};
DEFINE_REF(DescriptorSet)

class DescriptorPool : public Gfx::DescriptorPool {
public:
  DescriptorPool(PGraphics graphics, DescriptorLayout& layout);
  virtual ~DescriptorPool();
  NS::Array* getArguments() const
  {
    return layout.getArguments();
  }
private:
  PGraphics graphics;
  DescriptorLayout& layout;
};
} // namespace Metal
} // namespace Seele