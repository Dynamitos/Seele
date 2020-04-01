#include "VulkanGraphicsResources.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"
#include "VulkanGraphicsEnums.h"
#include <math.h>

using namespace Seele;
using namespace Seele::Vulkan;

TextureBase::TextureBase(PGraphics graphics, VkImageViewType viewType, uint32 sizeX, uint32 sizeY, uint32 sizeZ, 
    bool bArray, uint32 arraySize, uint32 mipLevel, 
    Gfx::SeFormat format, uint32 samples, Gfx::SeImageUsageFlags usage)
    : graphics(graphics)
    , sizeX(sizeX)
    , sizeY(sizeY)
    , sizeZ(sizeZ)
    , mipLevels(mipLevel)
    , format(format)
    , arrayCount(bArray ? arraySize : 1)
{
    PAllocator allocator = graphics->getAllocator();
    VkImageCreateInfo info =
        init::ImageCreateInfo();
    info.extent.width = sizeX;
    info.extent.height = sizeY;
    info.extent.depth = sizeZ;
    info.arrayLayers = arraySize;
    info.format = cast(format);
    
    VkImageViewCreateInfo viewInfo =
        init::ImageViewCreateInfo();
    viewInfo.subresourceRange = init::ImageSubresourceRange(aspect);
    viewInfo.viewType = viewType;

    uint32 layerCount = 1;
    switch (viewType)
    {
    case VK_IMAGE_VIEW_TYPE_1D:
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        info.imageType = VK_IMAGE_TYPE_1D;
        break;
    case VK_IMAGE_VIEW_TYPE_2D:
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        info.imageType = VK_IMAGE_TYPE_2D;
        break;
    case VK_IMAGE_VIEW_TYPE_3D:
        info.imageType = VK_IMAGE_TYPE_3D;
        break;
    case VK_IMAGE_VIEW_TYPE_CUBE:
        info.imageType = VK_IMAGE_TYPE_2D;
        layerCount = 6 * arrayCount;
        break;
    default:
        break;
    }
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.mipLevels = mipLevel;
    info.arrayLayers = arrayCount * layerCount;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.samples = (VkSampleCountFlagBits)samples;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usage;
    VK_CHECK(vkCreateImage(graphics->getDevice(), &info, nullptr, &image));

    viewInfo.image = image;
    viewInfo.format = cast(format);
    viewInfo.subresourceRange.layerCount = layerCount;
    viewInfo.subresourceRange.levelCount = mipLevels;

    VK_CHECK(vkCreateImageView(graphics->getDevice(), &viewInfo, nullptr, &defaultView));
}

TextureBase::~TextureBase()
{
    vkDestroyImageView(graphics->getDevice(), defaultView, nullptr);
    vkDestroyImage(graphics->getDevice(), image, nullptr);
}

Texture2D::Texture2D(PGraphics graphics, uint32 sizeX, uint32 sizeY, 
    bool bArray, uint32 arraySize, uint32 mipLevels, 
    Gfx::SeFormat format, uint32 samples, Gfx::SeImageUsageFlags usage)
{  
    textureHandle = new TextureBase(graphics, bArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, 
        sizeX, sizeY, 1, bArray, arraySize, mipLevels, format, samples, usage);

}

Texture2D::~Texture2D()
{
    
}