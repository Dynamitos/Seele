#pragma once
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "MinimalEngine.h"
#include "Buffer.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class DescriptorLayout : public Gfx::DescriptorLayout {
public:
  DescriptorLayout(PGraphics graphics, const std::string& name);
  virtual ~DescriptorLayout();
  virtual void create() override;

  NS::Array* getArguments() const { return arguments; }

private:
  PGraphics graphics;
  NS::Array* arguments;
};
DEFINE_REF(DescriptorLayout)

DECLARE_REF(DescriptorSet)
class DescriptorPool : public Gfx::DescriptorPool {
public:
  DescriptorPool(PGraphics graphics, PDescriptorLayout layout);
  virtual ~DescriptorPool();
  virtual Gfx::PDescriptorSet allocateDescriptorSet() override;
  virtual void reset() override;
  constexpr NS::Array* getArguments() const { return layout->getArguments(); }
  constexpr PDescriptorLayout getLayout() const { return layout; }

private:
  PGraphics graphics;
  PDescriptorLayout layout;
  Array<ODescriptorSet> allocatedSets;
};
DEFINE_REF(DescriptorPool)

class DescriptorSet : public Gfx::DescriptorSet {
public:
  DescriptorSet(PGraphics graphics, PDescriptorPool owner);
  virtual ~DescriptorSet();
  virtual void writeChanges();
  virtual void updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer);
  virtual void updateBuffer(uint32_t binding, Gfx::PShaderBuffer uniformBuffer);
  virtual void updateSampler(uint32_t binding, Gfx::PSampler samplerState);
  virtual void updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSampler sampler = nullptr);
  virtual void updateTextureArray(uint32_t binding, Array<Gfx::PTexture> texture);

  constexpr bool isCurrentlyBound() const { return bindCount > 0; }
  constexpr bool isCurrentlyInUse() const { return currentlyInUse; }
  constexpr void bind() { bindCount++; }
  constexpr void unbind() { bindCount--; }
  constexpr void allocate() { currentlyInUse = true; }
  constexpr void free() { currentlyInUse = false; }

  constexpr MTL::Buffer* getBuffer() const { return buffer; }
    constexpr const Array<MTL::Resource*>& getBoundResources() const { return boundResources; }

private:
  PGraphics graphics;
  PDescriptorPool owner;
  MTL::Buffer* buffer = nullptr;
  MTL::ArgumentEncoder* encoder;
    Array<MTL::Resource*> boundResources;
  uint32 bindCount;
  bool currentlyInUse;
};
DEFINE_REF(DescriptorSet)

class PipelineLayout : public Gfx::PipelineLayout {
public:
  PipelineLayout(PGraphics graphics, Gfx::PPipelineLayout baseLayout);
  virtual ~PipelineLayout();
  virtual void create() override;

private:
  PGraphics graphics;
};
DEFINE_REF(PipelineLayout)
} // namespace Metal
} // namespace Seele
