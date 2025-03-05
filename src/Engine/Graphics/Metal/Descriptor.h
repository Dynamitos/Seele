#pragma once
#include "Buffer.h"
#include "Foundation/NSArray.hpp"
#include "Foundation/NSObject.hpp"
#include "Graphics/Descriptor.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Command.h"
#include "Graphics/Metal/Resources.h"
#include "Graphics/Metal/Texture.h"
#include "Metal/MTLAccelerationStructure.hpp"
#include "Metal/MTLArgumentEncoder.hpp"
#include "Metal/MTLCommandEncoder.hpp"
#include "Metal/MTLLibrary.hpp"
#include "Metal/MTLResource.hpp"
#include "MinimalEngine.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
struct DescriptorMapping
{
    uint32 index;
    uint32 constantSize;
    MTL::ResourceUsage access;
};
class DescriptorLayout : public Gfx::DescriptorLayout {
  public:
    DescriptorLayout(PGraphics graphics, const std::string& name);
    virtual ~DescriptorLayout();
    virtual void create() override;
    MTL::ArgumentEncoder* createEncoder();
    constexpr bool isPlainDescriptor() const { return plainDescriptor; }
    
  private:
    constexpr uint64 flattenIndex(uint32 binding, uint32 arrayIndex) const { return uint64(arrayIndex) << 32 | binding; }
    PGraphics graphics;
    NS::Array* arguments;
    Map<std::string, DescriptorMapping> variableMapping;
    uint32 numResources;
    // descriptor sets containing only uniform data are not actually argument buffers, so they need to be
    // handled separately
    bool plainDescriptor = true;
    friend class DescriptorSet;
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
    virtual void reset();
    virtual void writeChanges() override;
    virtual void updateConstants(const std::string& name, uint32 offset, void* data) override;
    virtual void updateBuffer(const std::string& name, uint32 index, Gfx::PShaderBuffer uniformBuffer) override;
    virtual void updateBuffer(const std::string& name, uint32 index, Gfx::PVertexBuffer uniformBuffer) override;
    virtual void updateBuffer(const std::string& name, uint32 index, Gfx::PIndexBuffer uniformBuffer) override;
    virtual void updateSampler(const std::string& name, uint32 index, Gfx::PSampler samplerState) override;
    virtual void updateTexture(const std::string& name, uint32 index, Gfx::PTexture2D texture) override;
    virtual void updateTexture(const std::string& name, uint32 index, Gfx::PTexture3D texture) override;
    virtual void updateTexture(const std::string& name, uint32 index, Gfx::PTextureCube texture) override;
    virtual void updateAccelerationStructure(const std::string& name, uint32 index, Gfx::PTopLevelAS as) override;
    
    constexpr bool isPlainDescriptor() const { return owner->getLayout()->isPlainDescriptor(); }
    constexpr MTL::ArgumentEncoder* createEncoder() const { return owner->getLayout()->createEncoder(); }

  private:
    PGraphics graphics;
    PDescriptorPool owner;
    OBufferAllocation argumentBuffer = nullptr;
    MTL::ArgumentEncoder* encoder = nullptr;
    Array<PCommandBoundResource> boundResources;

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
        MTL::ResourceUsage access;
        PBufferAllocation buffer;
        void apply(MTL::ArgumentEncoder* encoder) const { encoder->setBuffer(buffer->buffer, 0, index); }
    };
    Array<BufferWriteInfo> bufferWrites;
    struct TextureWriteInfo
    {
        uint32 index;
        MTL::ResourceUsage access;
        PTextureHandle texture;
        void apply(MTL::ArgumentEncoder* encoder) const { encoder->setTexture(texture->texture, index); }
    };
    Array<TextureWriteInfo> textureWrites;
    struct SamplerWriteInfo
    {
        uint32 index;
        PSampler sampler;
        void apply(MTL::ArgumentEncoder* encoder) const { encoder->setSamplerState(sampler->getHandle(), index); }
    };
    Array<SamplerWriteInfo> samplerWrites;
    struct AccelerationStructureWriteInfo
    {
        uint32 index;
        MTL::ResourceUsage access;
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
