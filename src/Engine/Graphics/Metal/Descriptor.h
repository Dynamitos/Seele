#pragma once
#include "Buffer.h"
#include "Foundation/NSArray.hpp"
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Resources.h"
#include "Metal/MTLArgumentEncoder.hpp"
#include "Metal/MTLLibrary.hpp"
#include "MinimalEngine.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class DescriptorLayout : public Gfx::DescriptorLayout {
  public:
    DescriptorLayout(PGraphics graphics, const std::string& name);
    virtual ~DescriptorLayout();
    virtual void create() override;
    void setFunction(MTL::Function* func, uint64 ind) { function = func; index = ind; }
    MTL::ArgumentEncoder* createEncoder() { return function->newArgumentEncoder(index); }

  private:
    PGraphics graphics;
    MTL::Function* function;
    uint64 index;
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
    virtual void updateBuffer(uint32 binding, Gfx::PUniformBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32 binding, Gfx::PShaderBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32 binding, Gfx::PIndexBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PShaderBuffer uniformBuffer) override;
    virtual void updateSampler(uint32 binding, Gfx::PSampler samplerState) override;
    virtual void updateSampler(uint32 binding, uint32 dstArrayIndex, Gfx::PSampler samplerState) override;
    virtual void updateTexture(uint32 binding, Gfx::PTexture texture, Gfx::PSampler sampler = nullptr) override;
    virtual void updateTexture(uint32 binding, uint32 dstArrayIndex, Gfx::PTexture texture) override;
    virtual void updateTextureArray(uint32 binding, Array<Gfx::PTexture2D> texture) override;
    virtual void updateSamplerArray(uint32 binding, Array<Gfx::PSampler> samplers) override;
    virtual void updateAccelerationStructure(uint32 binding, Gfx::PTopLevelAS as) override;

    constexpr MTL::Buffer* getBuffer() const { return buffer; }
    constexpr const Array<Array<MTL::Resource*>>& getBoundResources() const { return boundResources; }

  private:
    PGraphics graphics;
    PDescriptorPool owner;
    MTL::ArgumentEncoder* encoder;
    MTL::Buffer* buffer = nullptr;
    Array<Array<MTL::Resource*>> boundResources;
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
