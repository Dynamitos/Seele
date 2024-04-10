#include "Texture.h"
#include "Enums.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Graphics.h"

using namespace Seele;
using namespace Seele::Metal;

TextureBase::TextureBase(PGraphics graphics, MTL::TextureType type,
                         const TextureCreateInfo &createInfo,
                         Gfx::QueueType &owner, MTL::Texture *existingImage)
    : currentOwner(owner), graphics(graphics), width(createInfo.width),
      height(createInfo.height), depth(createInfo.depth),
      arrayCount(createInfo.elements), layerCount(createInfo.layers),
      mipLevels(createInfo.mipLevels), samples(createInfo.samples),
      format(createInfo.format), usage(createInfo.usage),
      texture(existingImage), type(type),
      layout(Gfx::SE_IMAGE_LAYOUT_UNDEFINED),
      ownsImage(existingImage == nullptr) {
  if (existingImage == nullptr) {
    MTL::TextureUsage mtlUsage = 0;
    if(usage & Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        mtlUsage |= MTL::TextureUsageRenderTarget;
    }
    if(usage & Gfx::SE_IMAGE_USAGE_SAMPLED_BIT)
    {
        mtlUsage |= MTL::TextureUsageShaderRead;
    }
    MTL::TextureDescriptor *descriptor =
        MTL::TextureDescriptor::alloc()->init();
    descriptor->setPixelFormat(cast(format));
    descriptor->setWidth(width);
    descriptor->setHeight(height);
    descriptor->setDepth(depth);
    descriptor->setArrayLength(arrayCount);
    descriptor->setMipmapLevelCount(mipLevels);
    descriptor->setTextureType(type);
    descriptor->setSampleCount(samples);
    descriptor->setUsage(mtlUsage);
    
    texture = graphics->getDevice()->newTexture(descriptor);

    descriptor->release();
  }
}

TextureBase::~TextureBase() {
  if (ownsImage) {
    texture->release();
  }
}

void TextureBase::executePipelineBarrier(Gfx::SeAccessFlags,
                                         Gfx::SePipelineStageFlags,
                                         Gfx::SeAccessFlags,
                                         Gfx::SePipelineStageFlags) {}

void TextureBase::changeLayout(Gfx::SeImageLayout, Gfx::SeAccessFlags,
                               Gfx::SePipelineStageFlags, Gfx::SeAccessFlags,
                               Gfx::SePipelineStageFlags) {}

void TextureBase::download(uint32, uint32, uint32, Array<uint8>&)
{}

Texture2D::Texture2D(PGraphics graphics, const TextureCreateInfo &createInfo,
                     MTL::Texture *exisitingTexture)
    : Gfx::Texture2D(graphics->getFamilyMapping(), createInfo.sourceData.owner),
      TextureBase(graphics,
                  createInfo.elements > 1
                      ? (createInfo.samples > 1
                             ? MTL::TextureType2DMultisampleArray
                             : MTL::TextureType2DArray)
                      : (createInfo.samples > 1 ? MTL::TextureType2DMultisample
                                                : MTL::TextureType2D),
                  createInfo, Gfx::Texture2D::currentOwner, exisitingTexture) {}

Texture2D::~Texture2D()
{
}

void Texture2D::changeLayout(Gfx::SeImageLayout newLayout, 
        Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) 
{
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void Texture2D::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer)
{
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}

void Texture2D::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    TextureBase::executeOwnershipBarrier(newOwner);
}

void Texture2D::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, 
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) 
{
    TextureBase::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

Texture3D::Texture3D(PGraphics graphics, const TextureCreateInfo& createInfo)
    : Gfx::Texture3D(graphics->getFamilyMapping(), createInfo.sourceData.owner)
    , TextureBase(graphics, MTL::TextureType3D, createInfo, Gfx::Texture3D::currentOwner) {}


Texture3D::~Texture3D()
{
}

void Texture3D::changeLayout(Gfx::SeImageLayout newLayout, 
        Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) 
{
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void Texture3D::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer)
{
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}
void Texture3D::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    TextureBase::executeOwnershipBarrier(newOwner);
}

void Texture3D::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, 
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) 
{
    TextureBase::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

TextureCube::TextureCube(PGraphics graphics, const TextureCreateInfo& createInfo)
    : Gfx::TextureCube(graphics->getFamilyMapping(), createInfo.sourceData.owner)
    , TextureBase(graphics, createInfo.elements > 1 ? MTL::TextureTypeCubeArray : MTL::TextureTypeCube,
        createInfo, Gfx::TextureCube::currentOwner)
{
}

TextureCube::~TextureCube()
{
}

void TextureCube::changeLayout(Gfx::SeImageLayout newLayout, 
        Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) 
{
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void TextureCube::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer)
{
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}
void TextureCube::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    TextureBase::executeOwnershipBarrier(newOwner);
}

void TextureCube::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, 
        Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) 
{
    TextureBase::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
