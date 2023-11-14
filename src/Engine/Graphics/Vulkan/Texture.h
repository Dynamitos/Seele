#pragma once
#include "Graphics/Texture.h"
#include "Graphics.h"
#include "Allocator.h"

namespace Seele
{
namespace Vulkan
{

class TextureHandle
{
public:
    TextureHandle(PGraphics graphics, VkImageViewType viewType, 
        const TextureCreateInfo& createInfo, Gfx::QueueType& owner, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~TextureHandle();

    inline VkImage getImage() const
    {
        return image;
    }
    inline VkImageView getView() const
    {
        return defaultView;
    }
    inline Gfx::SeImageLayout getLayout() const
    {
        return layout;
    }
    inline VkImageAspectFlags getAspect() const
    {
        return aspect;
    }
    inline VkImageUsageFlags getUsage() const
    {
        return usage;
    }
    inline Gfx::SeFormat getFormat() const
    {
        return format;
    }
    inline Gfx::SeSampleCountFlags getNumSamples() const
    {
        return samples;
    }
    inline uint32 getMipLevels() const
    {
        return mipLevels;
    }
    inline bool isDepthStencil() const
    {
        return aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    }
    void executeOwnershipBarrier(Gfx::QueueType newOwner);
    void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
    void changeLayout(Gfx::SeImageLayout newLayout);
    void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer);

private:
    //Updates via reference
    Gfx::QueueType& currentOwner;
    PGraphics graphics;
    OSubAllocation allocation;
    uint32 sizeX;
    uint32 sizeY;
    uint32 sizeZ;
    uint32 arrayCount;
    uint32 layerCount;
    uint32 mipLevels;
    uint32 samples;
    Gfx::SeFormat format;
    Gfx::SeImageUsageFlags usage;
    VkImage image;
    VkImageView defaultView;
    VkImageAspectFlags aspect;
    Gfx::SeImageLayout layout;
    friend class TextureBase;
    friend class Texture2D;
    friend class Texture3D;
    friend class TextureCube;
    friend class Graphics;
};

class TextureBase
{
public:
    static TextureHandle* cast(Gfx::PTexture texture);

protected:
    TextureHandle* textureHandle;
    friend class Graphics;
};
DECLARE_REF(TextureBase)
class Texture2D : public Gfx::Texture2D, public TextureBase
{
public:
    Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~Texture2D();
    virtual uint32 getWidth() const override
    {
        return textureHandle->sizeX;
    }
    virtual uint32 getHeight() const override
    {
        return textureHandle->sizeY;
    }
    virtual uint32 getDepth() const override
    {
        return textureHandle->sizeZ;
    }
    virtual Gfx::SeFormat getFormat() const override
    {
        return textureHandle->format;
    }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override
    {
        return textureHandle->getNumSamples();
    }
    virtual uint32 getMipLevels() const override
    {
        return textureHandle->getMipLevels();
    }
    virtual void changeLayout(Gfx::SeImageLayout newLayout) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;
    virtual void* getNativeHandle() override
    {
        return textureHandle;
    }
    inline VkImage getHandle() const
    {
        return textureHandle->image;
    }
    inline VkImageView getView() const
    {
        return textureHandle->defaultView;
    }
    inline bool isDepthStencil() const
    {
        return textureHandle->isDepthStencil();
    }
protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);

};
DEFINE_REF(Texture2D)

class Texture3D : public Gfx::Texture3D, public TextureBase
{
public:
    Texture3D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~Texture3D();
    virtual uint32 getWidth() const override
    {
        return textureHandle->sizeX;
    }
    virtual uint32 getHeight() const override
    {
        return textureHandle->sizeY;
    }
    virtual uint32 getDepth() const override
    {
        return textureHandle->sizeZ;
    }
    virtual Gfx::SeFormat getFormat() const override
    {
        return textureHandle->format;
    }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override
    {
        return textureHandle->getNumSamples();
    }
    virtual uint32 getMipLevels() const override
    {
        return textureHandle->getMipLevels();
    }
    virtual void changeLayout(Gfx::SeImageLayout newLayout) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;
    virtual void* getNativeHandle() override
    {
        return textureHandle;
    }
    inline VkImage getHandle() const
    {
        return textureHandle->image;
    }
    inline VkImageView getView() const
    {
        return textureHandle->defaultView;
    }
    inline bool isDepthStencil() const
    {
        return textureHandle->isDepthStencil();
    }
protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);

};
DEFINE_REF(Texture3D)

class TextureCube : public Gfx::TextureCube, public TextureBase
{
public:
    TextureCube(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage = VK_NULL_HANDLE);
    virtual ~TextureCube();
    virtual uint32 getWidth() const override
    {
        return textureHandle->sizeX;
    }
    virtual uint32 getHeight() const override
    {
        return textureHandle->sizeY;
    }
    virtual uint32 getDepth() const override
    {
        return textureHandle->sizeZ;
    }
    virtual Gfx::SeFormat getFormat() const override
    {
        return textureHandle->format;
    }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override
    {
        return textureHandle->getNumSamples();
    }
    virtual uint32 getMipLevels() const override
    {
        return textureHandle->getMipLevels();
    }
    virtual void changeLayout(Gfx::SeImageLayout newLayout) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;
    virtual void* getNativeHandle() override
    {
        return textureHandle;
    }
    inline VkImage getHandle() const
    {
        return textureHandle->image;
    }
    inline VkImageView getView() const
    {
        return textureHandle->defaultView;
    }
    inline bool isDepthStencil() const
    {
        return textureHandle->isDepthStencil();
    }
protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner);
    virtual void executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);

};
DEFINE_REF(TextureCube)

}
}