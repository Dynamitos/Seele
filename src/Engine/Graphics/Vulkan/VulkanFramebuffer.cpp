#include "VulkanFramebuffer.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanInitializer.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphics.h"

using namespace Seele;
using namespace Seele::Vulkan;

Framebuffer::Framebuffer(PGraphics graphics, PRenderPass renderPass, Gfx::PRenderTargetLayout renderTargetLayout)
    : graphics(graphics)
    , layout(renderTargetLayout)
    , renderPass(renderPass)
{
    FramebufferDescription description;
    std::memset(&description, 0, sizeof(FramebufferDescription));
    Array<VkImageView> attachments;
    uint32 sizeX = 0;
    uint32 sizeY = 0;
    for (auto inputAttachment : layout->inputAttachments)
    {
        PTexture2D vkInputAttachment = inputAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkInputAttachment->getView());
        description.inputAttachments[description.numInputAttachments++] = vkInputAttachment->getView();
        sizeX = std::max(sizeX, vkInputAttachment->getSizeX());
        sizeY = std::max(sizeY, vkInputAttachment->getSizeY());
    }
    for (auto colorAttachment : layout->colorAttachments)
    {
        PTexture2D vkColorAttachment = colorAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkColorAttachment->getView());
        description.colorAttachments[description.numColorAttachments++] = vkColorAttachment->getView();
        sizeX = std::max(sizeX, vkColorAttachment->getSizeX());
        sizeY = std::max(sizeY, vkColorAttachment->getSizeY());
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D vkDepthAttachment = layout->depthAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkDepthAttachment->getView());
        description.depthAttachment = vkDepthAttachment->getView();
        sizeX = std::max(sizeX, vkDepthAttachment->getSizeX());
        sizeY = std::max(sizeY, vkDepthAttachment->getSizeY());
    }
    VkFramebufferCreateInfo createInfo =
        init::FramebufferCreateInfo(
            renderPass->getHandle(),
            (uint32)attachments.size(),
            attachments.data(),
            sizeX,
            sizeY,
            1);
    
    VK_CHECK(vkCreateFramebuffer(graphics->getDevice(), &createInfo, nullptr, &handle));

    boost::crc_32_type result;
    result.process_bytes(&description, sizeof(FramebufferDescription));
    hash = result.checksum();
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(graphics->getDevice(), handle, nullptr);
    graphics = nullptr;
}