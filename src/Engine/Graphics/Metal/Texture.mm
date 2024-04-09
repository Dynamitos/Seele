#include "Texture.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Graphics.h"
#include "Metal/MTLTexture.hpp"
#include "Enums.h"

using namespace Seele;
using namespace Seele::Metal;

TextureBase::TextureBase(PGraphics graphics, MTL::TextureType type, const TextureCreateInfo& createInfo, Gfx::QueueType& owner, MTL::Texture* existingImage)
    : currentOwner(owner)
    , graphics(graphics)
    , width(createInfo.width)
    , height(createInfo.height)
    , depth(createInfo.depth)
    , arrayCount(createInfo.elements)
    , layerCount(createInfo.layers)
    , mipLevels(createInfo.mipLevels)
    , samples(createInfo.samples)
    , format(createInfo.format)
    , usage(createInfo.usage)
    , texture(existingImage)
    , type(type)
    , layout(Gfx::SE_IMAGE_LAYOUT_UNDEFINED)
    , ownsImage(existingImage == nullptr)
{
    if(existingImage == nullptr)
    {
        MTL::TextureDescriptor* descriptor = MTL::TextureDescriptor::alloc()->init();
        descriptor->setPixelFormat(cast(format));
        descriptor->setWidth(width);
        descriptor->setHeight(height);
        descriptor->setDepth(depth);
        descriptor->setArrayLength(arrayCount);
        descriptor->setMipmapLevelCount(mipLevels);
        descriptor->setTextureType(type);
        descriptor->setSampleCount(samples);
        descriptor->setUsage(cast(usage));



        descriptor->release();
    }
}

Texture::~Texture()
{
    
}