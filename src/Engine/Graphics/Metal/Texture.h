#pragma once
#include "Graphics.h"
#include "Graphics/Enums.h"
#include "Graphics/Metal/Resources.h"
#include "Graphics/Texture.h"
#include "Metal/MTLTexture.hpp"

namespace Seele {
namespace Metal {
DECLARE_REF(TextureHandle)
class TextureView : public Gfx::TextureView, public CommandBoundResource {
  public:
    TextureView(PGraphics graphics, PTextureHandle source, uint32 width, uint32 height, uint32 numLayers, uint32 numMipLevels, MTL::Texture* view);
    virtual ~TextureView();
    virtual Gfx::SeFormat getFormat() const override;
    virtual uint32 getWidth() const override;
    virtual uint32 getHeight() const override;
    virtual uint32 getDepth() const override;
    virtual uint32 getNumLayers() const override;
    virtual Gfx::SeSampleCountFlags getNumSamples() const override;
    virtual uint32 getMipLevels() const override;
    virtual void pipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                 Gfx::SePipelineStageFlags dstStage) override;
    virtual void changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                              Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;

    MTL::Texture* getHandle() const { return view; }
    PTextureHandle getSource() const { return source;}

  private:
    uint32 width;
    uint32 height;
    uint32 numLayers;
    uint32 numMipLevels;
    PTextureHandle source;
    MTL::Texture* view;
    friend class TextureBase;
};
DEFINE_REF(TextureView)

class TextureHandle : public CommandBoundResource {
  public:
    TextureHandle(PGraphics graphics, MTL::TextureType type, const TextureCreateInfo& createInfo, MTL::Texture* existingImage);
    virtual ~TextureHandle();
    void pipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                         Gfx::SePipelineStageFlags dstStage);
    void transferOwnership(Gfx::QueueType newOwner);
    void changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                      Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage);
    void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer);
    void generateMipmaps();
    Gfx::OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount);

    MTL::Texture* texture;
    OTextureView textureView;
    MTL::TextureType type;
    uint32 width;
    uint32 height;
    uint32 depth;
    uint32 arrayCount;
    uint32 mipLevels;
    uint32 samples;
    Gfx::SeFormat format;
    Gfx::SeImageUsageFlags usage;
    Gfx::SeImageLayout layout;
    Gfx::SeImageAspectFlags aspect;
    uint8 ownsImage;
};
DEFINE_REF(TextureHandle)

class TextureBase {
  public:
    TextureBase(PGraphics graphics, MTL::TextureType textureType, const TextureCreateInfo& createInfo,
                MTL::Texture* existingImage = nullptr);
    virtual ~TextureBase();
    uint32 getWidth() const { return handle->width; }
    uint32 getHeight() const { return handle->height; }
    uint32 getDepth() const { return handle->depth; }
    uint32 getNumLayers() const { return handle->arrayCount; }
    PTextureHandle getHandle() const { return handle; }
    MTL::Texture* getImage() const { return handle->texture; };
    constexpr Gfx::SeImageLayout getLayout() const { return handle->layout; }
    void setLayout(Gfx::SeImageLayout val) { handle->layout = val; }
    constexpr Gfx::SeImageAspectFlags getAspect() const { return handle->aspect; }
    constexpr Gfx::SeImageUsageFlags getUsage() const { return handle->usage; }
    constexpr Gfx::SeFormat getFormat() const { return handle->format; }
    constexpr Gfx::SeSampleCountFlags getNumSamples() const { return handle->samples; }
    constexpr uint32 getMipLevels() const { return handle->mipLevels; }
    constexpr bool isDepthStencil() const { return handle->aspect & (Gfx::SE_IMAGE_ASPECT_DEPTH_BIT | Gfx::SE_IMAGE_ASPECT_STENCIL_BIT); }

    void pipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                         Gfx::SePipelineStageFlags dstStage);
    void transferOwnership(Gfx::QueueType newOwner);
    void changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                      Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage);
    void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer);
    void generateMipmaps();

  protected:
    OTextureHandle handle;
    // Updates via reference
    PGraphics graphics;
    Gfx::QueueType initialOwner;
    friend class Graphics;
};
DEFINE_REF(TextureBase)

class Texture2D : public Gfx::Texture2D, public TextureBase {
  public:
    Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, MTL::Texture* existingImage = nullptr);
    virtual ~Texture2D();
    virtual uint32 getWidth() const override { return handle->width; }
    virtual uint32 getHeight() const override { return handle->height; }
    virtual uint32 getDepth() const override { return handle->depth; }
    virtual uint32 getNumLayers() const override { return handle->arrayCount; }
    virtual Gfx::SeFormat getFormat() const override { return handle->format; }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override { return handle->samples; }
    virtual uint32 getMipLevels() const override { return handle->mipLevels; }
    virtual void changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                              Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;
    virtual void generateMipmaps() override;
    virtual Gfx::PTextureView getDefaultView() const override;
    virtual Gfx::OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(Texture2D)
class Texture2DArray : public Gfx::Texture2DArray, public TextureBase {
  public:
    Texture2DArray(PGraphics graphics, const TextureCreateInfo& createInfo, MTL::Texture* existingImage = nullptr);
    virtual ~Texture2DArray();
    virtual uint32 getWidth() const override { return handle->width; }
    virtual uint32 getHeight() const override { return handle->height; }
    virtual uint32 getDepth() const override { return handle->depth; }
    virtual uint32 getNumLayers() const override { return handle->arrayCount; }
    virtual Gfx::SeFormat getFormat() const override { return handle->format; }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override { return handle->samples; }
    virtual uint32 getMipLevels() const override { return handle->mipLevels; }
    virtual void changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                              Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;
    virtual void generateMipmaps() override;
    virtual Gfx::PTextureView getDefaultView() const override;
    virtual Gfx::OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(Texture2DArray)
class Texture3D : public Gfx::Texture3D, public TextureBase {
  public:
    Texture3D(PGraphics graphics, const TextureCreateInfo& createInfo);
    virtual ~Texture3D();
    virtual uint32 getWidth() const override { return handle->width; }
    virtual uint32 getHeight() const override { return handle->height; }
    virtual uint32 getDepth() const override { return handle->depth; }
    virtual uint32 getNumLayers() const override { return handle->arrayCount; }
    virtual Gfx::SeFormat getFormat() const override { return handle->format; }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override { return handle->samples; }
    virtual uint32 getMipLevels() const override { return handle->mipLevels; }
    virtual void changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                              Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;
    virtual void generateMipmaps() override;
    virtual Gfx::PTextureView getDefaultView() const override;
    virtual Gfx::OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(Texture3D)

class TextureCube : public Gfx::TextureCube, public TextureBase {
  public:
    TextureCube(PGraphics graphics, const TextureCreateInfo& createInfo);
    virtual ~TextureCube();
    virtual uint32 getWidth() const override { return handle->width; }
    virtual uint32 getHeight() const override { return handle->height; }
    virtual uint32 getDepth() const override { return handle->depth; }
    virtual uint32 getNumLayers() const override { return handle->arrayCount; }
    virtual Gfx::SeFormat getFormat() const override { return handle->format; }
    virtual Gfx::SeSampleCountFlags getNumSamples() const override { return handle->samples; }
    virtual uint32 getMipLevels() const override { return handle->mipLevels; }
    virtual void changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                              Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) override;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) override;
    virtual void generateMipmaps() override;
    virtual Gfx::PTextureView getDefaultView() const override;
    virtual Gfx::OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) override;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(Gfx::QueueType newOwner) override;
    virtual void executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                        Gfx::SePipelineStageFlags dstStage) override;
};
DEFINE_REF(TextureCube)
} // namespace Metal
} // namespace Seele
