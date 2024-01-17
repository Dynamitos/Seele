#pragma once
#include "Graphics/Texture.h"
#include "Graphics.h"

namespace Seele
{
namespace Vulkan
{
class TextureBase
{
public:
    TextureBase(PGraphics graphics, VkImageViewType viewType,
        const TextureCreateInfo& createInfo, Gfx::QueueType& owner, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~TextureBase();
    uint32 getWidth() const
    {
        return width;
    }
    uint32 getHeight() const
    {
        return height;
    }
    uint32 getDepth() const
    {
        return depth;
    }
    constexpr VkImage getImage() const
    {
        return image;
    }
    constexpr VkImageView getView() const
    {
        return imageView;
    }
    constexpr Gfx::SeImageLayout getLayout() const
    {
        return layout;
    }
    constexpr VkImageAspectFlags getAspect() const
    {
        return aspect;
    }
    constexpr VkImageUsageFlags getUsage() const
    {
        return usage;
    }
    constexpr Gfx::SeFormat getFormat() const
    {
        return format;
    }
    constexpr Gfx::SeSampleCountFlags getNumSamples() const
    {
        return samples;
    }
    constexpr uint32 getMipLevels() const
    {
        return mipLevels;
    }
    constexpr bool isDepthStencil() const
    {
        return aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    }
    void executeOwnershipBarrier(Gfx::QueueType newOwner);
    void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
    void changeLayout(Gfx::SeImageLayout newLayout);
    void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer);

protected:
    //Updates via reference
    Gfx::QueueType& currentOwner;
    PGraphics graphics;
    VmaAllocation allocation;
    uint32 width;
    uint32 height;
    uint32 depth;
    uint32 arrayCount;
    uint32 layerCount;
    uint32 mipLevels;
    uint32 samples;
    Gfx::SeFormat format;
    Gfx::SeImageUsageFlags usage;
    VkImage image;
    VkImageView imageView;
    VkImageAspectFlags aspect;
    Gfx::SeImageLayout layout;
    uint8 ownsImage;
    friend class Graphics;
};
DEFINE_REF(TextureBase)

class Texture2D : public Gfx::Texture2D, public TextureBase
{
public:
    Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~Texture2D();
    virtual uint32 getWidth() const override
    {
        return width;
    }
    virtual uint32 getHeight() const override
    {
        return height;
    }
    virtual uint32 getDepth() const override
    {
        return depth;
    }
    virtual Gfx::SeFormat getFormat() const override
    {
        return format;
    }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override
    {
        return samples;
    }
    virtual uint32 getMipLevels() const override
    {
        return mipLevels;
    }
    virtual void changeLayout(Gfx::SeImageLayout newLayout) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;

protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) override;

};
DEFINE_REF(Texture2D)

class Texture3D : public Gfx::Texture3D, public TextureBase
{
public:
    Texture3D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~Texture3D();
    virtual uint32 getWidth() const override
    {
        return width;
    }
    virtual uint32 getHeight() const override
    {
        return height;
    }
    virtual uint32 getDepth() const override
    {
        return depth;
    }
    virtual Gfx::SeFormat getFormat() const override
    {
        return format;
    }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override
    {
        return samples;
    }
    virtual uint32 getMipLevels() const override
    {
        return mipLevels;
    }
    virtual void changeLayout(Gfx::SeImageLayout newLayout) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;

protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) override;

};
DEFINE_REF(Texture3D)

class TextureCube : public Gfx::TextureCube, public TextureBase
{
public:
    TextureCube(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~TextureCube();
    virtual uint32 getWidth() const override
    {
        return width;
    }
    virtual uint32 getHeight() const override
    {
        return height;
    }
    virtual uint32 getDepth() const override
    {
        return depth;
    }
    virtual Gfx::SeFormat getFormat() const override
    {
        return format;
    }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override
    {
        return samples;
    }
    virtual uint32 getMipLevels() const override
    {
        return mipLevels;
    }
    virtual void changeLayout(Gfx::SeImageLayout newLayout) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;
protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) override;
};
DEFINE_REF(TextureCube)
}
}