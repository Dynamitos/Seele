#include "Texture.h"
#include "Command.h"
#include "Enums.h"
#include "Graphics/Enums.h"
#include "Graphics/Initializer.h"
#include <math.h>
#include <vulkan/vulkan_core.h>

using namespace Seele;
using namespace Seele::Vulkan;

VkImageAspectFlags getAspectFromFormat(Gfx::SeFormat format) {
    switch (format) {
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

TextureHandle::TextureHandle(PGraphics graphics, VkImageViewType viewType, const TextureCreateInfo& createInfo, VkImage existingImage)
    : CommandBoundResource(graphics), image(existingImage), width(createInfo.width), height(createInfo.height), depth(createInfo.depth),
      arrayCount(createInfo.elements), layerCount(createInfo.layers), mipLevels(createInfo.mipLevels), samples(createInfo.samples),
      format(createInfo.format), usage(createInfo.usage), layout(Gfx::SE_IMAGE_LAYOUT_UNDEFINED),
      aspect(getAspectFromFormat(createInfo.format)), ownsImage(false), owner(createInfo.sourceData.owner) {
    if (existingImage == VK_NULL_HANDLE) {
        VkImageType type = VK_IMAGE_TYPE_MAX_ENUM;
        VkImageCreateFlags flags = 0;
        switch (viewType) {
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
        if ((usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0) {
            usage = usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        VkImageCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .imageType = type,
            .format = cast(format),
            .extent =
                {
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
        ownsImage = true;
    }
    const DataSource& sourceData = createInfo.sourceData;
    if (sourceData.size > 0) {
        changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_NONE, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                     VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        void* data;
        VkBufferCreateInfo stagingInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = sourceData.size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        VmaAllocationCreateInfo alloc = {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        OBufferAllocation stagingAlloc = new BufferAllocation(graphics, createInfo.name, stagingInfo, alloc, Gfx::QueueType::TRANSFER);
        vmaMapMemory(graphics->getAllocator(), stagingAlloc->allocation, &data);
        std::memcpy(data, sourceData.data, sourceData.size);
        vmaUnmapMemory(graphics->getAllocator(), stagingAlloc->allocation);

        PCommandPool commandPool = graphics->getQueueCommands(owner);
        VkBufferImageCopy region = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = arrayCount * layerCount,
                },
            .imageOffset =
                {
                    .x = 0,
                    .y = 0,
                    .z = 0,
                },
            .imageExtent = {.width = width, .height = height, .depth = depth},
        };

        vkCmdCopyBufferToImage(commandPool->getCommands()->getHandle(), stagingAlloc->buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &region);

        commandPool->getCommands()->bindResource(PBufferAllocation(stagingAlloc));
        generateMipmaps();
        // When loading a texture from a file, we will almost always use it as a texture map for fragment shaders
        changeLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                     VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        graphics->getDestructionManager()->queueResourceForDestruction(std::move(stagingAlloc));
    }
    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image,
        .viewType = viewType,
        .format = cast(format),
        .subresourceRange =
            {
                .aspectMask = aspect,
                .levelCount = mipLevels,
                .layerCount = layerCount,
            },
    };

    VK_CHECK(vkCreateImageView(graphics->getDevice(), &viewInfo, nullptr, &imageView));
}

TextureHandle::~TextureHandle() {
    vkDestroyImageView(graphics->getDevice(), imageView, nullptr);
    if (ownsImage) {
        vmaDestroyImage(graphics->getAllocator(), image, allocation);
    }
}

void TextureHandle::pipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                    VkPipelineStageFlags dstStage) {
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
        .subresourceRange =
            {
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    PCommand command = graphics->getQueueCommands(owner)->getCommands();
    vkCmdPipelineBarrier(command->getHandle(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    command->bindResource(PTextureHandle(this));
}

void TextureHandle::transferOwnership(Gfx::QueueType newOwner) {
    Gfx::QueueFamilyMapping mapping = graphics->getFamilyMapping();
    if (mapping.getQueueTypeFamilyIndex(newOwner) == mapping.getQueueTypeFamilyIndex(owner))
        return;
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        .oldLayout = cast(layout),
        .newLayout = cast(layout),
        .srcQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(owner),
        .dstQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(newOwner),
        .image = image,
        .subresourceRange =
            {
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = arrayCount,
            },
    };
    PCommandPool sourcePool = graphics->getQueueCommands(owner);
    PCommandPool dstPool = graphics->getQueueCommands(newOwner);
    assert(barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex);
    vkCmdPipelineBarrier(sourcePool->getCommands()->getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);
    vkCmdPipelineBarrier(dstPool->getCommands()->getHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);
    sourcePool->submitCommands();
    owner = newOwner;
}

void TextureHandle::changeLayout(Gfx::SeImageLayout newLayout, VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
                                 VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) {
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
        .subresourceRange =
            {
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = layerCount,
            },
    };
    PCommandPool commandPool = graphics->getQueueCommands(owner);
    vkCmdPipelineBarrier(commandPool->getCommands()->getHandle(), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    commandPool->getCommands()->bindResource(PTextureHandle(this));
    layout = newLayout;
}

void TextureHandle::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    uint64 imageSize = width * height * depth * Gfx::getFormatInfo(format).blockSize;

    auto prevLayout = layout;
    changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                 VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    void* data;
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
    OBufferAllocation stagingAlloc = new BufferAllocation(graphics, "DownloadBuffer", stagingInfo, alloc, owner);

    PCommand cmdBuffer = graphics->getQueueCommands(owner)->getCommands();
    // Gfx::FormatCompatibilityInfo formatInfo = Gfx::getFormatInfo(format);
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = mipLevel,
                .baseArrayLayer = arrayLayer * layerCount + face,
                .layerCount = 1,
            },
        .imageOffset =
            {
                .x = 0,
                .y = 0,
                .z = 0,
            },
        .imageExtent = {.width = width, .height = height, .depth = depth},
    };
    vkCmdCopyImageToBuffer(cmdBuffer->getHandle(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingAlloc->buffer, 1, &region);
    cmdBuffer->bindResource(PBufferAllocation(stagingAlloc));
    changeLayout(prevLayout, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_MEMORY_READ_BIT,
                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    VK_CHECK(vmaMapMemory(graphics->getAllocator(), stagingAlloc->allocation, &data));
    buffer.resize(imageSize);
    std::memcpy(buffer.data(), data, buffer.size());
    vmaUnmapMemory(graphics->getAllocator(), stagingAlloc->allocation);
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(stagingAlloc));
}

void TextureHandle::generateMipmaps() {
    PCommandPool commandPool = graphics->getQueueCommands(owner);
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            {
                .aspectMask = aspect,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = layerCount,
            },
    };
    int32 mipWidth = width;
    int32 mipHeight = height;
    for (uint32 i = 1; i < mipLevels; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1;

        vkCmdPipelineBarrier(commandPool->getCommands()->getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit = {
            .srcSubresource =
                {
                    .aspectMask = aspect,
                    .mipLevel = i - 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .srcOffsets =
                {
                    {0, 0, 0},
                    {mipWidth, mipHeight, 1},
                },
            .dstSubresource = {.aspectMask = aspect, .mipLevel = i, .baseArrayLayer = 0, .layerCount = 1},
            .dstOffsets =
                {
                    {0, 0, 0},
                    {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1},
                },
        };
        vkCmdBlitImage(commandPool->getCommands()->getHandle(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                       aspect & VK_IMAGE_ASPECT_DEPTH_BIT ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight)
            mipHeight /= 2;
    }
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;

    // just so that all levels are in the same layout and we can transition them as one
    vkCmdPipelineBarrier(commandPool->getCommands()->getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);
    layout = Gfx::SE_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
}

TextureBase::TextureBase(PGraphics graphics, VkImageViewType viewType, const TextureCreateInfo& createInfo, VkImage existingImage)
    : handle(new TextureHandle(graphics, viewType, createInfo, existingImage)), graphics(graphics),
      initialOwner(createInfo.sourceData.owner) {}

TextureBase::~TextureBase() { graphics->getDestructionManager()->queueResourceForDestruction(std::move(handle)); }

void TextureBase::pipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                  VkPipelineStageFlags dstStage) {
    handle->pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
void TextureBase::transferOwnership(Gfx::QueueType newOwner) { handle->transferOwnership(newOwner); }
void TextureBase::changeLayout(Gfx::SeImageLayout newLayout, VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
                               VkAccessFlags dstAccess, VkPipelineStageFlags dstStage) {
    handle->changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}
void TextureBase::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    handle->download(mipLevel, arrayLayer, face, buffer);
}
void TextureBase::generateMipmaps() { handle->generateMipmaps(); }

Texture2D::Texture2D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage)
    : Gfx::Texture2D(graphics->getFamilyMapping()),
      TextureBase(graphics, createInfo.elements > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, createInfo, existingImage) {}

Texture2D::~Texture2D() {}

void Texture2D::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                             Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void Texture2D::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}

void Texture2D::generateMipmaps() { TextureBase::generateMipmaps(); }

void Texture2D::executeOwnershipBarrier(Gfx::QueueType newOwner) { TextureBase::transferOwnership(newOwner); }

void Texture2D::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                       VkPipelineStageFlags dstStage) {
    TextureBase::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

Texture3D::Texture3D(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage)
    : Gfx::Texture3D(graphics->getFamilyMapping()), TextureBase(graphics, VK_IMAGE_VIEW_TYPE_3D, createInfo, existingImage) {}

Texture3D::~Texture3D() {}

void Texture3D::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                             Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void Texture3D::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}

void Texture3D::generateMipmaps() { TextureBase::generateMipmaps(); }

void Texture3D::executeOwnershipBarrier(Gfx::QueueType newOwner) { TextureBase::transferOwnership(newOwner); }

void Texture3D::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                       VkPipelineStageFlags dstStage) {
    TextureBase::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

TextureCube::TextureCube(PGraphics graphics, const TextureCreateInfo& createInfo, VkImage existingImage)
    : Gfx::TextureCube(graphics->getFamilyMapping()),
      TextureBase(graphics, createInfo.elements > 1 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE, createInfo, existingImage) {}

TextureCube::~TextureCube() {}

void TextureCube::changeLayout(Gfx::SeImageLayout newLayout, Gfx::SeAccessFlags srcAccess, Gfx::SePipelineStageFlags srcStage,
                               Gfx::SeAccessFlags dstAccess, Gfx::SePipelineStageFlags dstStage) {
    TextureBase::changeLayout(newLayout, srcAccess, srcStage, dstAccess, dstStage);
}

void TextureCube::download(uint32 mipLevel, uint32 arrayLayer, uint32 face, Array<uint8>& buffer) {
    TextureBase::download(mipLevel, arrayLayer, face, buffer);
}

void TextureCube::generateMipmaps() { TextureBase::generateMipmaps(); }

void TextureCube::executeOwnershipBarrier(Gfx::QueueType newOwner) { TextureBase::transferOwnership(newOwner); }

void TextureCube::executePipelineBarrier(VkAccessFlags srcAccess, VkPipelineStageFlags srcStage, VkAccessFlags dstAccess,
                                         VkPipelineStageFlags dstStage) {
    TextureBase::pipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
