#pragma once
#include "Buffer.h"
#include "Foundation/NSArray.hpp"
#include "Foundation/NSObject.hpp"
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Command.h"
#include "Graphics/Metal/Resources.h"
#include "Metal/MTLAccelerationStructure.hpp"
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
    MTL::ArgumentEncoder* createEncoder();
    uint32 getFlattenedIndex(uint32 binding, uint32 arrayIndex) const { return flattenMap.at(flattenIndex(binding, arrayIndex)); }
    constexpr uint32 getTotalBindingCount() const { return flattenedBindingCount; }
    constexpr bool isPlainDescriptor() const { return plainDescriptor; }
    
  private:
    constexpr uint64 flattenIndex(uint32 binding, uint32 arrayIndex) const { return uint64(arrayIndex) << 32 | binding; }
    PGraphics graphics;
    NS::Array* arguments;
    Map<uint64, uint32> flattenMap;
    uint32 flattenedBindingCount = 0;
    // descriptor sets containing only uniform data are not actually argument buffers, so they need to be
    // handled separately
    bool plainDescriptor = true;
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
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PUniformBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PShaderBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PVertexBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PIndexBuffer uniformBuffer) override;
    virtual void updateSampler(uint32 binding, uint32 index, Gfx::PSampler samplerState) override;
    virtual void updateTexture(uint32 binding, uint32 index, Gfx::PTexture2D texture) override;
    virtual void updateTexture(uint32 binding, uint32 index, Gfx::PTexture3D texture) override;
    virtual void updateTexture(uint32 binding, uint32 index, Gfx::PTextureCube texture) override;
    virtual void updateAccelerationStructure(uint32 binding, uint32 index, Gfx::PTopLevelAS as) override;
    
    constexpr const Array<MTL::Resource*>& getBoundResources() const { return boundResources; }
    constexpr bool isPlainDescriptor() const { return owner->getLayout()->isPlainDescriptor(); }
    constexpr MTL::ArgumentEncoder* createEncoder() const { return owner->getLayout()->createEncoder(); }

  private:
    PGraphics graphics;
    PDescriptorPool owner;
    Array<MTL::Resource*> boundResources;
    MTL::Buffer* argumentBuffer = nullptr;
    MTL::ArgumentEncoder* encoder = nullptr;

    struct UniformWriteInfo
    {
        uint32 index;
        Array<uint8> content;
        void apply(MTL::ArgumentEncoder* encoder) const { std::memcpy(encoder->constantData(index), content.data(), content.size()); }
    };
    Array<UniformWriteInfo> uniformWrites;
    struct BufferWriteInfo
    {
        uint32 index;
        MTL::Buffer* buffer;
        void apply(MTL::ArgumentEncoder* encoder) const { encoder->setBuffer(buffer, 0, 2); }
    };
    Array<BufferWriteInfo> bufferWrites;
    struct TextureWriteInfo
    {
        uint32 index;
        MTL::Texture* texture;
        void apply(MTL::ArgumentEncoder* encoder) const { encoder->setTexture(texture, index); }
    };
    Array<TextureWriteInfo> textureWrites;
    struct SamplerWriteInfo
    {
        uint32 index;
        MTL::SamplerState* sampler;
        void apply(MTL::ArgumentEncoder* encoder) const { encoder->setSamplerState(sampler, index); }
    };
    Array<SamplerWriteInfo> samplerWrites;
    struct AccelerationStructureWriteInfo
    {
        uint32 index;
        MTL::AccelerationStructure* accelerationStructure;
        void apply(MTL::ArgumentEncoder* encoder) const { encoder->setAccelerationStructure(accelerationStructure, index); }
    };
    Array<AccelerationStructureWriteInfo> accelerationWrites;
    
    friend class RenderCommand;
    friend class ComputeCommand;
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
