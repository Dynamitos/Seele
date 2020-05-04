#include "VulkanGraphicsResources.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanAllocator.h"
#include "VulkanCommandBuffer.h"
#include <math.h>

using namespace Seele;
using namespace Seele::Vulkan;

VkImageAspectFlags getAspectFromFormat(Gfx::SeFormat format)
{
    switch (format)
    {
    case Gfx::SE_FORMAT_D16_UNORM:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case Gfx::SE_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    case Gfx::SE_FORMAT_S8_UINT:
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    case Gfx::SE_FORMAT_D16_UNORM_S8_UINT:
    case Gfx::SE_FORMAT_D24_UNORM_S8_UINT:
    case Gfx::SE_FORMAT_D32_SFLOAT_S8_UINT:
    case Gfx::SE_FORMAT_X8_D24_UNORM_PACK32:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

TextureHandle::TextureHandle(PGraphics graphics, VkImageViewType viewType, uint32 sizeX, uint32 sizeY, uint32 sizeZ,
                             bool bArray, uint32 arraySize, uint32 mipLevel,
                             Gfx::SeFormat format, uint32 samples, Gfx::SeImageUsageFlags usage, Gfx::QueueType owner, VkImage existingImage)
    : QueueOwnedResource(graphics, owner), graphics(graphics), sizeX(sizeX), sizeY(sizeY), sizeZ(sizeZ), mipLevels(mipLevel), format(format), samples(samples), usage(usage), arrayCount(bArray ? arraySize : 1), aspect(getAspectFromFormat(format)), image(existingImage), layout(VK_IMAGE_LAYOUT_UNDEFINED)
{
    if (existingImage == VK_NULL_HANDLE)
    {
        PAllocator allocator = graphics->getAllocator();
        VkImageCreateInfo info =
            init::ImageCreateInfo();
        info.extent.width = sizeX;
        info.extent.height = sizeY;
        info.extent.depth = sizeZ;
        info.arrayLayers = arraySize;
        info.format = cast(format);

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
        case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
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

        VkMemoryDedicatedRequirements memDedicatedRequirements;
        memDedicatedRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
        memDedicatedRequirements.pNext = nullptr;
        VkImageMemoryRequirementsInfo2 reqInfo;
        reqInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
        reqInfo.pNext = nullptr;
        reqInfo.image = image;
        VkMemoryRequirements2 requirements;
        requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        requirements.pNext = &memDedicatedRequirements;
        vkGetImageMemoryRequirements2(graphics->getDevice(), &reqInfo, &requirements);

        allocation = allocator->allocate(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);
        vkBindImageMemory(graphics->getDevice(), image, allocation->getHandle(), allocation->getOffset());
    }
    VkImageViewCreateInfo viewInfo =
        init::ImageViewCreateInfo();
    viewInfo.subresourceRange = init::ImageSubresourceRange(aspect);
    viewInfo.viewType = viewType;
    viewInfo.image = image;
    viewInfo.format = cast(format);
    viewInfo.subresourceRange.layerCount = (viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY || viewType == VK_IMAGE_VIEW_TYPE_CUBE) ? 6 : 1;
    viewInfo.subresourceRange.levelCount = mipLevels;

    VK_CHECK(vkCreateImageView(graphics->getDevice(), &viewInfo, nullptr, &defaultView));
}

TextureHandle::~TextureHandle()
{
    auto &deletionQueue = graphics->getDeletionQueue();
    auto fence = getCommands()->getCommands()->getFence();
    VkDevice device = graphics->getDevice();
    VkImageView view = defaultView;
    VkImage img = image;
    deletionQueue.addPendingDelete(fence, [device, view]() { vkDestroyImageView(device, view, nullptr); });
    deletionQueue.addPendingDelete(fence, [device, img]() { vkDestroyImage(device, img, nullptr); });
}

void TextureHandle::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    VkImageMemoryBarrier imageBarrier =
        init::ImageMemoryBarrier();
    imageBarrier.image = image;
    imageBarrier.oldLayout = layout;
    imageBarrier.newLayout = layout;
    imageBarrier.subresourceRange = init::ImageSubresourceRange(aspect);
    PCommandBufferManager sourceManager = getCommands();
    PCommandBufferManager dstManager = nullptr;
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    QueueFamilyMapping mapping = graphics->getFamilyMapping();
    imageBarrier.srcQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(currentOwner);
    imageBarrier.dstQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(newOwner);
    if (currentOwner == Gfx::QueueType::TRANSFER || currentOwner == Gfx::QueueType::DEDICATED_TRANSFER)
    {
        imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (currentOwner == Gfx::QueueType::COMPUTE)
    {
        imageBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if (currentOwner == Gfx::QueueType::GRAPHICS)
    {
        imageBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }
    if (newOwner == Gfx::QueueType::TRANSFER)
    {
        imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstManager = graphics->getTransferCommands();
    }
    else if (currentOwner == Gfx::QueueType::DEDICATED_TRANSFER)
    {
        imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstManager = graphics->getDedicatedTransferCommands();
    }
    else if (newOwner == Gfx::QueueType::COMPUTE)
    {
        imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dstManager = graphics->getComputeCommands();
    }
    else if (newOwner == Gfx::QueueType::GRAPHICS)
    {
        imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        dstManager = graphics->getGraphicsCommands();
    }
    VkCommandBuffer sourceCmd = sourceManager->getCommands()->getHandle();
    VkCommandBuffer destCmd = dstManager->getCommands()->getHandle();
    vkCmdPipelineBarrier(sourceCmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    vkCmdPipelineBarrier(destCmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    sourceManager->submitCommands();
    cachedCmdBufferManager = dstManager;
}

void TextureBase::changeLayout(VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier =
        init::ImageMemoryBarrier(
            textureHandle->image,
            textureHandle->layout,
            newLayout);
    vkCmdPipelineBarrier(textureHandle->getCommands()->getCommands()->getHandle(),
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);
    textureHandle->layout = newLayout;
}

Texture2D::Texture2D(PGraphics graphics, uint32 sizeX, uint32 sizeY,
                     bool bArray, uint32 arraySize, uint32 mipLevels, Gfx::SeFormat format,
                     uint32 samples, Gfx::SeImageUsageFlags usage, Gfx::QueueType owner, VkImage existingImage)
{
    textureHandle = new TextureHandle(graphics, bArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
                                      sizeX, sizeY, 1, bArray, arraySize, mipLevels, format, samples, usage, owner, existingImage);
}

Texture2D::~Texture2D()
{
}