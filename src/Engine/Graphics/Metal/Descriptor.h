#pragma once
#include "Buffer.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Resources.h"
#include "MinimalEngine.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class DescriptorLayout : public Gfx::DescriptorLayout {
  public:
    DescriptorLayout(PGraphics graphics, const std::string& name);
    virtual ~DescriptorLayout();
    virtual void create() override;

  private:
    PGraphics graphics;
};
DEFINE_REF(DescriptorLayout)

DECLARE_REF(DescriptorSet)
class DescriptorPool : public Gfx::DescriptorPool {
  public:
    DescriptorPool(PGraphics graphics, PDescriptorLayout layout);
    virtual ~DescriptorPool();
    virtual Gfx::PDescriptorSet allocateDescriptorSet() override;
    virtual void reset() override;
    constexpr PDescriptorLayout getLayout() const { return layout; }

  private:
    PGraphics graphics;
    PDescriptorLayout layout;
    Array<ODescriptorSet> allocatedSets;
};
DEFINE_REF(DescriptorPool)

class DescriptorSet : public Gfx::DescriptorSet, public CommandBoundResource {
  public:
    DescriptorSet(PGraphics graphics, PDescriptorPool owner);
    virtual ~DescriptorSet();
    virtual void writeChanges() override;
    virtual void updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32_t binding, Gfx::PShaderBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32_t binding, uint32 index, Gfx::PShaderBuffer uniformBuffer) override;
    virtual void updateSampler(uint32_t binding, Gfx::PSampler samplerState) override;
    virtual void updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSampler sampler = nullptr) override;
    virtual void updateTextureArray(uint32_t binding, Array<Gfx::PTexture> texture) override;

    constexpr bool isCurrentlyInUse() const { return currentlyInUse; }
    constexpr void allocate() { currentlyInUse = true; }
    constexpr void free() { currentlyInUse = false; }

    constexpr MTL::Buffer* getBuffer() const { return buffer; }
    constexpr const Array<MTL::Resource*>& getBoundResources() const { return boundResources; }

  private:
    PGraphics graphics;
    PDescriptorPool owner;
    MTL::Buffer* buffer = nullptr;
    uint64* argumentBuffer = nullptr;
    Array<MTL::Resource*> boundResources;
    bool currentlyInUse;
};
DEFINE_REF(DescriptorSet)

class PipelineLayout : public Gfx::PipelineLayout {
  public:
    PipelineLayout(PGraphics graphics, const std::string& name, Gfx::PPipelineLayout baseLayout);
    virtual ~PipelineLayout();
    virtual void create() override;

  private:
    PGraphics graphics;
};
DEFINE_REF(PipelineLayout)
} // namespace Metal
} // namespace Seele
