#pragma once
#include "Containers/List.h"
#include "Enums.h"
#include "Graphics/Descriptor.h"
#include "Resources.h"

namespace Seele {
namespace Vulkan {
DECLARE_REF(Graphics)
class DescriptorLayout : public Gfx::DescriptorLayout {
  public:
    DescriptorLayout(PGraphics graphics, const std::string& name);
    virtual ~DescriptorLayout();
    virtual void create() override;
    constexpr VkDescriptorSetLayout getHandle() const { return layoutHandle; }

  private:
    PGraphics graphics;
    Array<VkDescriptorSetLayoutBinding> bindings;
    VkDescriptorSetLayout layoutHandle;
    friend class DescriptorPool;
};
DEFINE_REF(DescriptorLayout)

DECLARE_REF(DescriptorSet)
class DescriptorPool : public Gfx::DescriptorPool, public CommandBoundResource {
  public:
    DescriptorPool(PGraphics graphics, PDescriptorLayout layout);
    virtual ~DescriptorPool();
    virtual Gfx::PDescriptorSet allocateDescriptorSet() override;
    virtual void reset() override;

    constexpr VkDescriptorPool getHandle() const { return poolHandle; }
    constexpr PDescriptorLayout getLayout() const { return layout; }

  private:
    PGraphics graphics;
    PDescriptorLayout layout;
    const static int maxSets = 64;
    StaticArray<ODescriptorSet, maxSets> cachedHandles;
    VkDescriptorPool poolHandle;
    DescriptorPool* nextAlloc = nullptr;
};
DEFINE_REF(DescriptorPool)

class DescriptorSet : public Gfx::DescriptorSet, public CommandBoundResource {
  public:
    DescriptorSet(PGraphics graphics, PDescriptorPool owner);
    virtual ~DescriptorSet();
    virtual void writeChanges() override;
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PUniformBuffer uniformBuffer) override;
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PShaderBuffer shaderBuffer) override;
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PVertexBuffer indexBuffer) override;
    virtual void updateBuffer(uint32 binding, uint32 index, Gfx::PIndexBuffer indexBuffer) override;
    virtual void updateSampler(uint32 binding, uint32 index, Gfx::PSampler samplerState) override;
    virtual void updateTexture(uint32 binding, uint32 index, Gfx::PTexture2D texture) override;
    virtual void updateTexture(uint32 binding, uint32 index, Gfx::PTexture3D texture) override;
    virtual void updateTexture(uint32 binding, uint32 index, Gfx::PTextureCube texture) override;
    virtual void updateAccelerationStructure(uint32 binding, uint32 index, Gfx::PTopLevelAS as) override;

    constexpr VkDescriptorSet getHandle() const { return setHandle; }

  private:
    List<VkDescriptorImageInfo> imageInfos;
    List<VkDescriptorBufferInfo> bufferInfos;
    List<VkWriteDescriptorSetAccelerationStructureKHR> accelerationInfos;
    Array<VkWriteDescriptorSet> writeDescriptors;
    // contains the previously bound resources at every binding
    // since the layout is fixed, trying to bind a texture to a buffer
    // would not work anyways, so casts should be safe
    // Array<void*> cachedData;
    Array<Array<PCommandBoundResource>> boundResources;
    VkDescriptorSet setHandle;
    PGraphics graphics;
    PDescriptorPool owner;
    friend class DescriptorPool;
    friend class Command;
    friend class RenderCommand;
    friend class ComputeCommand;
};
DEFINE_REF(DescriptorSet)

class PipelineLayout : public Gfx::PipelineLayout {
  public:
    PipelineLayout(PGraphics graphics, const std::string& name, Gfx::PPipelineLayout baseLayout);
    virtual ~PipelineLayout();
    virtual void create();
    constexpr VkPipelineLayout getHandle() const { return layoutHandle; }

  private:
    Array<VkDescriptorSetLayout> vulkanDescriptorLayouts;
    PGraphics graphics;
    VkPipelineLayout layoutHandle;
};
DEFINE_REF(PipelineLayout)

} // namespace Vulkan
} // namespace Seele