#pragma once
#include "Graphics/Texture.h"
#include "Graphics.h"
#include "Metal/MTLTexture.hpp"

namespace Seele
{
namespace Metal
{
class TextureBase
{
public:
    TextureBase(PGraphics graphics, MTL::TextureType type, const TextureCreateInfo& createInfo, Gfx::QueueType& owner, MTL::Texture* existingImage = nullptr);
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
    constexpr MTL::Texture* getTexture() const
    {
        return texture;
    }
    constexpr Gfx::SeImageLayout getLayout() const
    {
        return layout;
    }
    void setLayout(Gfx::SeImageLayout val)
    {
        layout = val;
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
    void executeOwnershipBarrier(Gfx::QueueType newOwner) { currentOwner = newOwner; }
    void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage);
    void changeLayout(Gfx::SeImageLayout newLayout, 
        Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage);
    void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer);

protected:
    //Updates via reference
    Gfx::QueueType& currentOwner;
    PGraphics graphics;
    uint32 width;
    uint32 height;
    uint32 depth;
    uint32 arrayCount;
    uint32 layerCount;
    uint32 mipLevels;
    uint32 samples;
    Gfx::SeFormat format;
    Gfx::SeImageUsageFlags usage;
    MTL::Texture* texture;
    MTL::TextureType type;
    Gfx::SeImageLayout layout;
    uint8 ownsImage;
    friend class Graphics;
};
DEFINE_REF(TextureBase)
class Texture2D : public Gfx::Texture2D
{
public:
    Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, MTL::Texture* existingImage = nullptr);
    virtual ~Texture2D();

    virtual Gfx::SeFormat getFormat() const override;
    virtual uint32 getWidth() const override;
    virtual uint32 getHeight() const override;
    virtual uint32 getDepth() const override;
    virtual Gfx::SeSampleCountFlags getNumSamples() const override;
    virtual uint32 getMipLevels() const override;
    virtual void changeLayout(Gfx::SeImageLayout newLayout, 
        Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
protected:
    //Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, 
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(Texture2D)
class Texture3D : public Gfx::Texture3D
{

};
DEFINE_REF(Texture3D)
class TextureCube : public Gfx::TextureCube
{

};
DEFINE_REF(TextureCube)
}
}