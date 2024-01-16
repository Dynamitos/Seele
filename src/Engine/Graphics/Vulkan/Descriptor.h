#pragma once
#include "Enums.h"
#include "Graphics/Descriptor.h"
#include "Containers/List.h"
#include "Resources.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Graphics)
class DescriptorLayout : public Gfx::DescriptorLayout
{
public:
    DescriptorLayout(PGraphics graphics, const std::string& name);
    virtual ~DescriptorLayout();
    virtual void create();
    constexpr VkDescriptorSetLayout getHandle() const
    {
        return layoutHandle;
    }
private:
    uint32 hash;
    PGraphics graphics;
    Array<VkDescriptorSetLayoutBinding> bindings;
    VkDescriptorSetLayout layoutHandle;
    friend class DescriptorPool;
};
DEFINE_REF(DescriptorLayout)
class PipelineLayout : public Gfx::PipelineLayout
{
public:
    PipelineLayout(PGraphics graphics, Gfx::PPipelineLayout baseLayout)
        : Gfx::PipelineLayout(baseLayout)
        , graphics(graphics)
        , layoutHandle(VK_NULL_HANDLE)
    {}
    virtual ~PipelineLayout();
    virtual void create();
    virtual void reset();
    constexpr VkPipelineLayout getHandle() const
    {
        return layoutHandle;
    }

private:
    Array<VkDescriptorSetLayout> vulkanDescriptorLayouts;
    PGraphics graphics;
    VkPipelineLayout layoutHandle;
};
DEFINE_REF(PipelineLayout)

class DescriptorSet : public Gfx::DescriptorSet
{
public:
    DescriptorSet(PGraphics graphics, PDescriptorPool owner)
        : setHandle(VK_NULL_HANDLE)
        , graphics(graphics)
        , owner(owner)
        , bindCount(0)
        , currentlyInUse(false)
    {
    }
    virtual ~DescriptorSet();
    virtual void writeChanges();
    virtual void updateBuffer(uint32_t binding, Gfx::PUniformBuffer uniformBuffer);
    virtual void updateBuffer(uint32_t binding, Gfx::PShaderBuffer uniformBuffer);
    virtual void updateSampler(uint32_t binding, Gfx::PSampler samplerState);
    virtual void updateTexture(uint32_t binding, Gfx::PTexture texture, Gfx::PSampler sampler = nullptr);
    virtual void updateTextureArray(uint32_t binding, Array<Gfx::PTexture> texture);
    virtual bool operator<(Gfx::PDescriptorSet other);
    
    constexpr bool isCurrentlyBound() const
    {
        return bindCount > 0;
    }
    constexpr bool isCurrentlyInUse() const
    {
        return currentlyInUse;
    }
    constexpr void bind()
    {
        bindCount++;
    }
    constexpr void unbind()
    {
        bindCount--;
    }
    constexpr void allocate()
    {
        currentlyInUse = true;
    }
    constexpr void free()
    {
        currentlyInUse = false;
    }
    constexpr VkDescriptorSet getHandle() const
    {
        return setHandle;
    }
    virtual uint32 getSetIndex() const;

private:
    List<VkDescriptorImageInfo> imageInfos;
    List<VkDescriptorBufferInfo> bufferInfos;
    Array<VkWriteDescriptorSet> writeDescriptors;
    // contains the previously bound resources at every binding
    // since the layout is fixed, trying to bind a texture to a buffer
    // would not work anyways, so casts should be safe
    Array<void*> cachedData;
    VkDescriptorSet setHandle;
    PGraphics graphics;
    PDescriptorPool owner;
    uint32 bindCount;
    bool currentlyInUse;
    friend class DescriptorPool;
    friend class Command;
    friend class RenderCommand;
    friend class ComputeCommand;
};
DEFINE_REF(DescriptorSet)

class DescriptorPool : public Gfx::DescriptorPool
{
public:
    DescriptorPool(PGraphics graphics, DescriptorLayout &layout);
    virtual ~DescriptorPool();
    virtual Gfx::PDescriptorSet allocateDescriptorSet() override;
    virtual void reset() override;

    constexpr VkDescriptorPool getHandle() const
    {
        return poolHandle;
    }
    constexpr DescriptorLayout& getLayout() const
    {
        return layout;
    }

private:
    PGraphics graphics;
    DescriptorLayout &layout;
    const static int maxSets = 64;
    StaticArray<ODescriptorSet, maxSets> cachedHandles;
    VkDescriptorPool poolHandle;
    DescriptorPool* nextAlloc = nullptr;
};
DEFINE_REF(DescriptorPool)
} // namespace Vulkan
} // namespace Seele