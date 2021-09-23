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

TextureHandle::TextureHandle(PGraphics graphics, VkImageViewType viewType, 
    const TextureCreateInfo& createInfo, Gfx::QueueType& owner, VkImage existingImage)
    : currentOwner(owner)
    , graphics(graphics)
    , sizeX(createInfo.width)
    , sizeY(createInfo.height)
    , sizeZ(createInfo.depth)
    , arrayCount(createInfo.bArray ? createInfo.arrayLayers : 1)
    , mipLevels(createInfo.mipLevels)
    , samples(createInfo.samples)
    , format(createInfo.format)
    , usage(createInfo.usage)
    , image(existingImage)
    , aspect(getAspectFromFormat(createInfo.format))
    , layout(Gfx::SE_IMAGE_LAYOUT_UNDEFINED)
{
    if (existingImage == VK_NULL_HANDLE)
    {
        PAllocator allocator = graphics->getAllocator();
        VkImageCreateInfo info =
            init::ImageCreateInfo();
        info.extent.width = sizeX;
        info.extent.height = sizeY;
        info.extent.depth = sizeZ;
        info.arrayLayers = arrayCount;
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

        info.initialLayout = cast(layout);
        info.mipLevels = mipLevels;
        info.arrayLayers = arrayCount * layerCount;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.samples = (VkSampleCountFlagBits)samples;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = usage;
        // Most of these flags will almost always be used
        info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        
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

    const BulkResourceData& resourceData = createInfo.resourceData;
    if(resourceData.size > 0)
    {
        changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        PStagingBuffer staging = graphics->getStagingManager()->allocateStagingBuffer(resourceData.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        void* data = staging->getMappedPointer();
        std::memcpy(data, resourceData.data, resourceData.size);
        staging->flushMappedMemory();
        
        PCommandBufferManager cmdBufferManager = graphics->getQueueCommands(currentOwner);
        VkBufferImageCopy region;
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {sizeX, sizeY, sizeZ};

        vkCmdCopyBufferToImage(cmdBufferManager->getCommands()->getHandle(), 
            staging->getHandle(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        
        // When loading a texture from a file, we will almost always use it as a texture map for fragment shaders
        changeLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    else if(usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        changeLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
    else
    {
        changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL);
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
    auto cmdBuffer = graphics->getQueueCommands(currentOwner)->getCommands();
    VkDevice device = graphics->getDevice();
    VkImageView view = defaultView;
    VkImage img = image;
    deletionQueue.addPendingDelete(cmdBuffer, [device, view]() { vkDestroyImageView(device, view, nullptr); });
    deletionQueue.addPendingDelete(cmdBuffer, [device, img]() { vkDestroyImage(device, img, nullptr); });
}

TextureHandle* TextureBase::cast(Gfx::PTexture texture) 
{
    if(texture->getTexture2D() != nullptr)
    {
        PTexture2D texture2D = texture.cast<Texture2D>();
        return texture2D->textureHandle;
    }
    return nullptr;
}

void TextureHandle::changeLayout(Gfx::SeImageLayout newLayout)
{
    VkImageMemoryBarrier barrier =
        init::ImageMemoryBarrier(
            image,
            cast(layout),
            cast(newLayout));
    barrier.subresourceRange =
        init::ImageSubresourceRange(aspect);
    PCommandBufferManager cmdManager = graphics->getQueueCommands(currentOwner);
    vkCmdPipelineBarrier(cmdManager->getCommands()->getHandle(),
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            0, 0, nullptr, 0, nullptr, 1, &barrier);
    cmdManager->submitCommands();
    layout = newLayout;
}

void TextureHandle::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    VkImageMemoryBarrier imageBarrier =
        init::ImageMemoryBarrier();
    imageBarrier.image = image;
    imageBarrier.oldLayout = cast(layout);
    imageBarrier.newLayout = cast(layout);
    imageBarrier.subresourceRange = init::ImageSubresourceRange(aspect);
    PCommandBufferManager sourceManager = graphics->getQueueCommands(currentOwner);
    PCommandBufferManager dstManager = nullptr;
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    Gfx::QueueFamilyMapping mapping = graphics->getFamilyMapping();
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
    else if (newOwner == Gfx::QueueType::DEDICATED_TRANSFER)
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
    vkCmdPipelineBarrier(sourceCmd, srcStage, srcStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    vkCmdPipelineBarrier(destCmd, dstStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    currentOwner = newOwner;
    sourceManager->submitCommands();
}

void TextureHandle::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    VkImageMemoryBarrier imageBarrier =
        init::ImageMemoryBarrier(
            image,
            cast(layout),
            cast(layout)
        );
    imageBarrier.srcAccessMask = srcAccess;
    imageBarrier.dstAccessMask = dstAccess;
    imageBarrier.subresourceRange = init::ImageSubresourceRange(aspect);
    PCmdBuffer cmdBuffer = graphics->getQueueCommands(currentOwner)->getCommands();
    vkCmdPipelineBarrier(cmdBuffer->getHandle(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
}

Texture2D::Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage)
    : Gfx::Texture2D(graphics->getFamilyMapping(), createInfo.resourceData.owner)
{
    textureHandle = new TextureHandle(graphics, createInfo.bArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, 
        createInfo, currentOwner, existingImage);
}

Texture2D::~Texture2D()
{
}

void Texture2D::changeLayout(Gfx::SeImageLayout newLayout) 
{
    textureHandle->changeLayout(newLayout);
}

void Texture2D::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    textureHandle->executeOwnershipBarrier(newOwner);
}

void Texture2D::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    textureHandle->executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}