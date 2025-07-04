#pragma once
#include "Resources.h"

namespace Seele {
namespace Gfx {

class TextureView {
public:
    TextureView();
    virtual ~TextureView();
    virtual SeFormat getFormat() const = 0;
    virtual uint32 getWidth() const = 0;
    virtual uint32 getHeight() const = 0;
    virtual uint32 getDepth() const = 0;
    virtual uint32 getNumLayers() const = 0; 
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout, SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                              SePipelineStageFlags dstStage) = 0;
    virtual void pipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess, SePipelineStageFlags dstStage) = 0;
private:
};
DEFINE_REF(TextureView)

class Texture : public QueueOwnedResource {
  public:
    Texture(QueueFamilyMapping mapping);
    virtual ~Texture();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getWidth() const = 0;
    virtual uint32 getHeight() const = 0;
    virtual uint32 getDepth() const = 0;
    virtual uint32 getNumLayers() const = 0; 
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout, SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                              SePipelineStageFlags dstStage) = 0;
    virtual void download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) = 0;
    virtual void generateMipmaps() = 0;

    virtual PTextureView getDefaultView() const = 0;
    virtual OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) = 0;
  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                                        SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(Texture)

class Texture2D : public Texture {
  public:
    Texture2D(QueueFamilyMapping mapping);
    virtual ~Texture2D();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getWidth() const = 0;
    virtual uint32 getHeight() const = 0;
    virtual uint32 getDepth() const = 0;
    virtual uint32 getNumLayers() const = 0; 
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout, SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                              SePipelineStageFlags dstStage) = 0;
    virtual void generateMipmaps() = 0;
    virtual PTextureView getDefaultView() const = 0;
    
    virtual OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) = 0;
  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                                        SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(Texture2D)

class Texture3D : public Texture {
  public:
    Texture3D(QueueFamilyMapping mapping);
    virtual ~Texture3D();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getWidth() const = 0;
    virtual uint32 getHeight() const = 0;
    virtual uint32 getDepth() const = 0;
    virtual uint32 getNumLayers() const = 0;
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout, SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                              SePipelineStageFlags dstStage) = 0;
    virtual void generateMipmaps() = 0;
    virtual PTextureView getDefaultView() const = 0;
    virtual OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) = 0;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                                        SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(Texture3D)

class TextureCube : public Texture {
  public:
    TextureCube(QueueFamilyMapping mapping);
    virtual ~TextureCube();

    virtual SeFormat getFormat() const = 0;
    virtual uint32 getWidth() const = 0;
    virtual uint32 getHeight() const = 0;
    virtual uint32 getDepth() const = 0;
    virtual uint32 getNumLayers() const = 0;
    virtual SeSampleCountFlags getNumSamples() const = 0;
    virtual uint32 getMipLevels() const = 0;
    virtual void changeLayout(SeImageLayout newLayout, SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                              SePipelineStageFlags dstStage) = 0;
    virtual void generateMipmaps() = 0;
    virtual PTextureView getDefaultView() const = 0;
    virtual OTextureView createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) = 0;

  protected:
    // Inherited via QueueOwnedResource
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                                        SePipelineStageFlags dstStage) = 0;
};
DEFINE_REF(TextureCube)
} // namespace Gfx
} // namespace Seele