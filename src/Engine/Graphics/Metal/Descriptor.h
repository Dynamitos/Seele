#pragma once
#include "Buffer.h"
#include "Foundation/NSArray.hpp"
#include "Foundation/NSObject.hpp"
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Resources.h"
#include "Metal/MTLArgumentEncoder.hpp"
#include "Metal/MTLLibrary.hpp"
#include "Metal/MTLResource.hpp"
#include "MinimalEngine.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class DescriptorLayout : public Gfx::DescriptorLayout {
  public:
    DescriptorLayout(PGraphics graphics, const std::string& name);
    virtual ~DescriptorLayout();
    virtual void create() override;
    void setFunction(MTL::Function* func, uint32 ind){function = func; index = ind;}
    MTL::ArgumentEncoder* createEncoder();

  private:
    PGraphics graphics;
    NS::Array* arguments;
    MTL::Function* function;
    uint32 index = 0;
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
    virtual void updateSampler(uint32 binding, Gfx::PSampler samplerState) override;
    virtual void updateTexture(uint32 binding, Gfx::PTexture texture, Gfx::PSampler sampler = nullptr) override;
    virtual void updateTextureArray(uint32 binding, Array<Gfx::PTexture2D> texture) override;
    virtual void updateSamplerArray(uint32 binding, Array<Gfx::PSampler> samplers) override;
    virtual void updateAccelerationStructure(uint32 binding, Gfx::PTopLevelAS as) override;

    constexpr const Array<MTL::Resource*>& getBoundResources() const { return boundResources; }

    MTL::Buffer* getArgumentBuffer() const { return argumentBuffer; }

  private:
    PGraphics graphics;
    PDescriptorPool owner;
    Array<MTL::Resource*> boundResources;
    MTL::ArgumentEncoder* encoder;
    MTL::Buffer* argumentBuffer = nullptr;
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
