#include "Texture.h"
#include "Command.h"
#include "Enums.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Buffer.h"
#include "Graphics/Metal/Graphics.h"
#include "Graphics/Metal/Resources.h"
#include "Metal/MTLTexture.hpp"
#include "Metal/MTLTypes.hpp"

using namespace Seele;
using namespace Seele::Metal;

TextureView::TextureView(PGraphics graphics, PTextureHandle source, uint32 width, uint32 height, uint32 numLayers, uint32 numMipLevels, MTL::Texture* view)
    : CommandBoundResource(graphics), width(width), height(height), numLayers(numLayers), numMipLevels(numMipLevels), source(source), view(view) {}

TextureView::~TextureView() {}

Gfx::SeFormat TextureView::getFormat() const { return source->format; }

uint32 TextureView::getWidth() const { return width; }

uint32 TextureView::getHeight() const { return height; }

uint32 TextureView::getDepth() const { return source->depth; }

uint32 TextureView::getNumLayers() const { return numLayers; }

Gfx::SeSampleCountFlags TextureView::getNumSamples() const { return source->samples; }

uint32 TextureView::getMipLevels() const { return numMipLevels; }

void TextureView::pipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                  Gfx::SePipelineStageFlags dstStage) {
    return source->pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

void TextureView::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                               Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    return source->changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

Gfx::OTextureView TextureHandle::createTextureView(uint32 baseMipLevel, uint32 viewLevelCount, uint32 baseArrayLayer,
                                                   uint32 viewLayerCount) {
    MTL::Texture* viewTexture =
        texture->newTextureView(cast(format), type, NS::Range(baseMipLevel, viewLevelCount), NS::Range(baseArrayLayer, viewLayerCount));
    uint32 viewWidth = width * std::pow(0.5, baseMipLevel);
    uint32 viewHeight = height * std::pow(0.5, baseMipLevel);
    return new TextureView(graphics, this, viewWidth, viewHeight, viewLayerCount, viewLevelCount, viewTexture);
}

TextureHandle::TextureHandle(PGraphics graphics, MTL::TextureType type, const TextureCreateInfo& createInfo, MTL::Texture* existingImage)
    : CommandBoundResource(graphics), texture(existingImage), type(type), width(createInfo.width), height(createInfo.height),
      depth(createInfo.depth), arrayCount(createInfo.elements), mipLevels(1), samples(createInfo.samples), format(createInfo.format),
      usage(createInfo.usage), layout(Gfx::SE_IMAGE_LAYOUT_UNDEFINED), ownsImage(existingImage == nullptr) {
    if (createInfo.useMip) {
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    }
    if (existingImage == nullptr) {
        MTL::TextureUsage mtlUsage = 0;
        if (usage & Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT || usage & Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            mtlUsage |= MTL::TextureUsageRenderTarget;
        }
        if (usage & Gfx::SE_IMAGE_USAGE_SAMPLED_BIT) {
            mtlUsage |= MTL::TextureUsageShaderRead;
        }
        if (usage & Gfx::SE_IMAGE_USAGE_STORAGE_BIT) {
            mtlUsage |= MTL::TextureUsageShaderWrite;
        }
        MTL::TextureDescriptor* descriptor = MTL::TextureDescriptor::alloc()->init();
        descriptor->setPixelFormat(cast(format));
        descriptor->setWidth(width);
        descriptor->setHeight(height);
        descriptor->setDepth(depth);
        descriptor->setArrayLength(arrayCount);
        descriptor->setMipmapLevelCount(mipLevels);
        descriptor->setTextureType(type);
        descriptor->setSampleCount(samples);
        descriptor->setUsage(mtlUsage);
        descriptor->setStorageMode(MTL::StorageModePrivate);
        if (usage & Gfx::SE_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
            descriptor->setStorageMode(MTL::StorageModeMemoryless);
        }

        texture = graphics->getDevice()->newTexture(descriptor);
        texture->setLabel(NS::String::string(createInfo.name.c_str(), NS::ASCIIStringEncoding));

        descriptor->release();
    }
    if (createInfo.sourceData.data != nullptr) {
        OBufferAllocation stagingBuffer =
            new BufferAllocation(graphics, "TextureStaging", createInfo.sourceData.size, MTL::ResourceStorageModeShared);
        std::memcpy(stagingBuffer->map(), createInfo.sourceData.data, createInfo.sourceData.size);
        MTL::BlitCommandEncoder* blitEnc = graphics->getQueue()->getCommands()->getBlitEncoder();
        uint32 sliceSize = createInfo.sourceData.size / arrayCount;
        uint32 numSlices = arrayCount;
        if (type == MTL::TextureTypeCube || type == MTL::TextureTypeCubeArray) {
            sliceSize /= 6;
            numSlices *= 6;
        }
        uint32 offset = 0;
        for (uint32 slice = 0; slice < numSlices; ++slice) {
            blitEnc->copyFromBuffer(stagingBuffer->buffer, offset, sliceSize / createInfo.height, arrayCount == 1 ? 0 : sliceSize,
                                    MTL::Size(createInfo.width, createInfo.height, createInfo.depth), texture, slice, 0, MTL::Origin());
            offset += sliceSize;
        }
        if (mipLevels > 1) {
            blitEnc->generateMipmaps(texture);
        }
        graphics->getQueue()->getCommands()->bindResource(PBufferAllocation(stagingBuffer));
        graphics->getDestructionManager()->queueResourceForDestruction(std::move(stagingBuffer));
    }
    textureView = new TextureView(graphics, this, width, height, arrayCount, mipLevels, texture);
}

TextureHandle::~TextureHandle() {
    if (ownsImage) {
        texture->release();
    }
}

void TextureHandle::pipelineBarrier(Gfx::SeAccessFlags, Gfx::SePipelineStageFlags, Gfx::SeAccessFlags, Gfx::SePipelineStageFlags) {}

void TextureHandle::changeLayout(Gfx::SeImageLayout, Gfx::SeAccessFlags, Gfx::SePipelineStageFlags, Gfx::SeAccessFlags,
                                 Gfx::SePipelineStageFlags) {}

void TextureHandle::transferOwnership(Gfx::QueueType) {}

void TextureHandle::download(uint32, uint32, uint32, Array<uint8>&) {}

void TextureHandle::generateMipmaps() {}

TextureBase::TextureBase(PGraphics graphics, MTL::TextureType viewType, const TextureCreateInfo& createInfo, MTL::Texture* existingImage)
    : handle(new TextureHandle(graphics, viewType, createInfo, existingImage)), graphics(graphics) {}

TextureBase::~TextureBase() { graphics->getDestructionManager()->queueResourceForDestruction(std::move(handle)); }

void TextureBase::pipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                  Gfx::SePipelineStageFlags dstStage) {
    handle->pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
void TextureBase::transferOwnership(Gfx::QueueType newOwner) { handle->transferOwnership(newOwner); }
void TextureBase::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                               Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    handle->changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}
void TextureBase::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    handle->download(mipLevel, arrayLayer, face, buffer);
}
void TextureBase::generateMipmaps() { handle->generateMipmaps(); }

Texture2D::Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, MTL::Texture* existingImage)
    : Gfx::Texture2D(graphics->getFamilyMapping()),
      TextureBase(graphics,
                  createInfo.elements > 1 ? (createInfo.samples > 1 ? MTL::TextureType2DMultisampleArray : MTL::TextureType2DArray)
                                          : (createInfo.samples > 1 ? MTL::TextureType2DMultisample : MTL::TextureType2D),
                  createInfo, existingImage) {}

Texture2D::~Texture2D() {}

void Texture2D::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                             Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void Texture2D::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}

void Texture2D::generateMipmaps() { TextureBase::generateMipmaps(); }

Gfx::PTextureView Texture2D::getDefaultView() const { return PTextureView(handle->textureView); }

Gfx::OTextureView Texture2D::createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) {
    return handle->createTextureView(baseMipLevel, levelCount, baseArrayLayer, layerCount);
}

void Texture2D::executeOwnershipBarrier(Gfx::QueueType newOwner) { TextureBase::transferOwnership(newOwner); }

void Texture2D::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                       Gfx::SePipelineStageFlags dstStage) {
    TextureBase::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

Texture2DArray::Texture2DArray(PGraphics graphics, const TextureCreateInfo& createInfo, MTL::Texture* existingImage)
    : Gfx::Texture2DArray(graphics->getFamilyMapping()),
      TextureBase(graphics,
                  createInfo.elements > 1 ? (createInfo.samples > 1 ? MTL::TextureType2DMultisampleArray : MTL::TextureType2DArray)
                                          : (createInfo.samples > 1 ? MTL::TextureType2DMultisample : MTL::TextureType2D),
                  createInfo, existingImage) {}

Texture2DArray::~Texture2DArray() {}

void Texture2DArray::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                                  Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void Texture2DArray::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}

void Texture2DArray::generateMipmaps() { TextureBase::generateMipmaps(); }

Gfx::PTextureView Texture2DArray::getDefaultView() const { return PTextureView(handle->textureView); }

Gfx::OTextureView Texture2DArray::createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) {
    return handle->createTextureView(baseMipLevel, levelCount, baseArrayLayer, layerCount);
}

void Texture2DArray::executeOwnershipBarrier(Gfx::QueueType newOwner) { TextureBase::transferOwnership(newOwner); }

void Texture2DArray::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                            Gfx::SePipelineStageFlags dstStage) {
    TextureBase::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

Texture3D::Texture3D(PGraphics graphics, const TextureCreateInfo& createInfo)
    : Gfx::Texture3D(graphics->getFamilyMapping()), TextureBase(graphics, MTL::TextureType3D, createInfo) {}

Texture3D::~Texture3D() {}

void Texture3D::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                             Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void Texture3D::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}

void Texture3D::generateMipmaps() { TextureBase::generateMipmaps(); }

Gfx::PTextureView Texture3D::getDefaultView() const { return PTextureView(handle->textureView); }

Gfx::OTextureView Texture3D::createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) {
    return handle->createTextureView(baseMipLevel, levelCount, baseArrayLayer, layerCount);
}

void Texture3D::executeOwnershipBarrier(Gfx::QueueType newOwner) { TextureBase::transferOwnership(newOwner); }

void Texture3D::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                       Gfx::SePipelineStageFlags dstStage) {
    TextureBase::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

TextureCube::TextureCube(PGraphics graphics, const TextureCreateInfo& createInfo)
    : Gfx::TextureCube(graphics->getFamilyMapping()),
      TextureBase(graphics, createInfo.elements > 1 ? MTL::TextureTypeCubeArray : MTL::TextureTypeCube, createInfo) {}

TextureCube::~TextureCube() {}

void TextureCube::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                               Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void TextureCube::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}

void TextureCube::generateMipmaps() { TextureBase::generateMipmaps(); }

Gfx::PTextureView TextureCube::getDefaultView() const { return PTextureView(handle->textureView); }

Gfx::OTextureView TextureCube::createTextureView(uint32 baseMipLevel, uint32 levelCount, uint32 baseArrayLayer, uint32 layerCount) {
    return handle->createTextureView(baseMipLevel, levelCount, baseArrayLayer, layerCount);
}

void TextureCube::executeOwnershipBarrier(Gfx::QueueType newOwner) { TextureBase::transferOwnership(newOwner); }

void TextureCube::executePipelineBarrier(Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage, Gfx::SeAccessFlags dstAccess,
                                         Gfx::SePipelineStageFlags dstStage) {
    TextureBase::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
