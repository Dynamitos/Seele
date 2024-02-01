#include "Texture.h"
#include "Command.h"
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

TextureBase::TextureBase(PGraphics graphics, VkImageViewType viewType,
    const TextureCreateInfo& createInfo, Gfx::QueueType& owner, VkImage existingImage)
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
    , image(existingImage)
    , aspect(getAspectFromFormat(createInfo.format))
    , layout(Gfx::SE_IMAGE_LAYOUT_UNDEFINED)
    , ownsImage(false)
{
    if (existingImage == VK_NULL_HANDLE)
    {
        ownsImage = true;
        VkImageType type = VK_IMAGE_TYPE_MAX_ENUM;
        VkImageCreateFlags flags = 0;
        switch (viewType)
        {
        case VK_IMAGE_VIEW_TYPE_1D:
        case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
            type = VK_IMAGE_TYPE_1D;
            break;
        case VK_IMAGE_VIEW_TYPE_2D:
        case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
            type = VK_IMAGE_TYPE_2D;
            break;
        case VK_IMAGE_VIEW_TYPE_3D:
            type = VK_IMAGE_TYPE_3D;
            break;
        case VK_IMAGE_VIEW_TYPE_CUBE:
        case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
            type = VK_IMAGE_TYPE_2D;
            flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            layerCount = 6 * arrayCount;
            break;
        default:
            break;
        }
        if ((usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0)
        {
            usage = usage
                | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                | VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        VkImageCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .imageType = type,
            .format = cast(format),
            .extent = {
                .width = width,
                .height = height,
                .depth = depth,
            },
            .mipLevels = mipLevels,
            .arrayLayers = arrayCount * layerCount,
            .samples = (VkSampleCountFlagBits)samples,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = cast(layout),
        };
        VmaAllocationCreateInfo allocInfo = {
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        VK_CHECK(vmaCreateImage(graphics->getAllocator(), &info, &allocInfo, &image, &allocation, nullptr));
        
        const DataSource& sourceData = createInfo.sourceData;
        if(sourceData.size > 0)
        {
            changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_ACCESS_NONE, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
            void* data;
            VkMemoryPropertyFlags memProps;
            VkBuffer stagingBuffer = VK_NULL_HANDLE;
            VmaAllocation stagingAlloc = VK_NULL_HANDLE;
            vmaGetAllocationMemoryProperties(graphics->getAllocator(), allocation, &memProps);
            if(memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vmaMapMemory(graphics->getAllocator(), allocation, &data);
            }
            else
            {
                VkBufferCreateInfo stagingInfo = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .size = sourceData.size,
                    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                };
                VmaAllocationCreateInfo alloc = {
                    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    .usage = VMA_MEMORY_USAGE_AUTO,
                };
                VK_CHECK(vmaCreateBuffer(graphics->getAllocator(), &stagingInfo, &alloc, &stagingBuffer, &stagingAlloc, nullptr));
                vmaMapMemory(graphics->getAllocator(), stagingAlloc, &data);
            }
            std::memcpy(data, sourceData.data, sourceData.size);
            vmaFlushAllocation(graphics->getAllocator(), stagingAlloc, 0, VK_WHOLE_SIZE);

            PCommandPool commandPool = graphics->getQueueCommands(currentOwner);
            VkBufferImageCopy region = {
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = arrayCount * layerCount,
                },
                .imageOffset = {
                    .x = 0, 
                    .y = 0, 
                    .z = 0,
                },
                .imageExtent = {
                    .width = width, 
                    .height = height, 
                    .depth = depth
                },
            };
            
            vkCmdCopyBufferToImage(commandPool->getCommands()->getHandle(), 
                stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            
            // When loading a texture from a file, we will almost always use it as a texture map for fragment shaders
            changeLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            if(stagingBuffer != VK_NULL_HANDLE)
            {
                vmaUnmapMemory(graphics->getAllocator(), stagingAlloc);
                graphics->getDestructionManager()->queueBuffer(commandPool->getCommands(), stagingBuffer, stagingAlloc);
            }
        }
    }
    if(usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        changeLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_ACCESS_NONE, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    }
    else if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        changeLayout(Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_ACCESS_NONE, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }
    else
    {
        changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_NONE, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    }
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image,
        .viewType = viewType,
        .format = cast(format),
        .subresourceRange = {
            .aspectMask = aspect,
            .levelCount = mipLevels,
            .layerCount = layerCount,
        },
    };
    
    VK_CHECK(vkCreateImageView(graphics->getDevice(), &viewInfo, nullptr, &imageView));
}

TextureBase::~TextureBase()
{
    if (ownsImage)
    {
        graphics->getDestructionManager()->queueImage(graphics->getQueueCommands(currentOwner)->getCommands(), image, allocation);
    }
    graphics->getDestructionManager()->queueImageView(graphics->getQueueCommands(currentOwner)->getCommands(), imageView);
}


void TextureBase::changeLayout(Gfx::SeImageLayout newLayout, 
        VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage)
{
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = srcAccess,
        .dstAccessMask = dstAccess,
        .oldLayout = cast(layout),
        .newLayout = cast(newLayout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = layerCount,
        },
    };
    PCommandPool commandPool = graphics->getQueueCommands(currentOwner);
    vkCmdPipelineBarrier(commandPool->getCommands()->getHandle(),
                            srcStage, dstStage,
                            0, 0, nullptr, 0, nullptr, 1, &barrier);
    commandPool->submitCommands();
    layout = newLayout;
}

void TextureBase::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer)
{
    uint64 imageSize = width * height * depth * Gfx::getFormatInfo(format).blockSize;

    auto prevLayout = layout;
    changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    void* data;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAlloc;
    // always create a staging buffer since we do buffer -> image copy and the image may be in any tiling format
    VkBufferCreateInfo stagingInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = imageSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VmaAllocationCreateInfo alloc = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    VK_CHECK(vmaCreateBuffer(graphics->getAllocator(), &stagingInfo, &alloc, &stagingBuffer, &stagingAlloc, nullptr));

    PCommand cmdBuffer = graphics->getQueueCommands(currentOwner)->getCommands();
    //Gfx::FormatCompatibilityInfo formatInfo = Gfx::getFormatInfo(format);
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = mipLevel,
            .baseArrayLayer = arrayLayer * layerCount + face,
            .layerCount = 1,
        },
        .imageOffset = { 
            .x = 0, 
            .y = 0, 
            .z = 0,
        },
        .imageExtent = { 
            .width = width,
            .height = height, 
            .depth = depth
        },
    };
    vkCmdCopyImageToBuffer(cmdBuffer->getHandle(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
    changeLayout(prevLayout,
        VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    VK_CHECK(vmaMapMemory(graphics->getAllocator(), stagingAlloc, &data));
    buffer.resize(imageSize);
    std::memcpy(buffer.data(), data, buffer.size());
    vmaUnmapMemory(graphics->getAllocator(), stagingAlloc);
    graphics->getDestructionManager()->queueBuffer(cmdBuffer, stagingBuffer, stagingAlloc);
}

void TextureBase::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
    Gfx::QueueFamilyMapping mapping = graphics->getFamilyMapping();
    VkImageMemoryBarrier imageBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .oldLayout = cast(layout),
        .newLayout = cast(layout),
        .srcQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(currentOwner),
        .dstQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(newOwner),
        .image = image,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = arrayCount,
        },
    };
    PCommandPool sourcePool = graphics->getQueueCommands(currentOwner);
    PCommandPool dstPool = nullptr;
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    if (currentOwner == Gfx::QueueType::TRANSFER)
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
        dstPool = graphics->getTransferCommands();
    }
    else if (newOwner == Gfx::QueueType::COMPUTE)
    {
        imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dstPool = graphics->getComputeCommands();
    }
    else if (newOwner == Gfx::QueueType::GRAPHICS)
    {
        imageBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        dstPool = graphics->getGraphicsCommands();
    }
    
    VkCommandBuffer sourceCmd = sourcePool->getCommands()->getHandle();
    VkCommandBuffer destCmd = dstPool->getCommands()->getHandle();
    vkCmdPipelineBarrier(sourceCmd, srcStage, srcStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    vkCmdPipelineBarrier(destCmd, dstStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    currentOwner = newOwner;
    sourcePool->submitCommands();
}

void TextureBase::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = srcAccess,
        .dstAccessMask = dstAccess,
        .oldLayout = cast(layout),
        .newLayout = cast(layout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    PCommand command = graphics->getQueueCommands(currentOwner)->getCommands();
    vkCmdPipelineBarrier(command->getHandle(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

Texture2D::Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage)
    : Gfx::Texture2D(graphics->getFamilyMapping(), createInfo.sourceData.owner)
    , TextureBase(graphics, createInfo.elements > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
        createInfo, Gfx::Texture2D::currentOwner, existingImage)
{
}

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

void Texture2D::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    TextureBase::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

Texture3D::Texture3D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage)
    : Gfx::Texture3D(graphics->getFamilyMapping(), createInfo.sourceData.owner)
    , TextureBase(graphics, VK_IMAGE_VIEW_TYPE_3D,
        createInfo, Gfx::Texture3D::currentOwner, existingImage)
{
}

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

void Texture3D::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    TextureBase::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

TextureCube::TextureCube(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage)
    : Gfx::TextureCube(graphics->getFamilyMapping(), createInfo.sourceData.owner)
    , TextureBase(graphics, createInfo.elements > 1? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE,
        createInfo, Gfx::TextureCube::currentOwner, existingImage)
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

void TextureCube::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, 
        VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) 
{
    TextureBase::executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
