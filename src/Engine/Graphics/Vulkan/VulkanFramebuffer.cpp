#include "VulkanFramebuffer.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanInitializer.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphics.h"

using namespace Seele;
using namespace Seele::Vulkan;

Framebuffer::Framebuffer(PGraphics graphics, PRenderPass renderPass, Gfx::PRenderTargetLayout renderTargetLayout)
    : graphics(graphics), layout(renderTargetLayout), renderPass(renderPass)
{
    FramebufferDescription description;
    std::memset(&description, 0, sizeof(FramebufferDescription));
    Array<VkImageView> attachments;
    for (auto inputAttachment : layout->inputAttachments)
    {
        PTexture2D vkInputAttachment = inputAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkInputAttachment->getView());
        description.inputAttachments[description.numInputAttachments++] = vkInputAttachment->getView();
    }
    for (auto colorAttachment : layout->colorAttachments)
    {
        PTexture2D vkColorAttachment = colorAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkColorAttachment->getView());
        description.colorAttachments[description.numColorAttachments++] = vkColorAttachment->getView();
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D vkDepthAttachment = layout->depthAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkDepthAttachment->getView());
        description.depthAttachment = vkDepthAttachment->getView();
    }
    VkFramebufferCreateInfo createInfo =
        init::FramebufferCreateInfo(
            renderPass->getHandle(),
            attachments.size(),
            attachments.data(),
            renderPass->getRenderArea().extent.width,
            renderPass->getRenderArea().extent.height,
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