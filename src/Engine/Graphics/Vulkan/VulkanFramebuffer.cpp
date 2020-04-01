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
    Array<VkImageView> attachments;
    for(auto inputAttachment : layout->inputAttachments)
    {
        PTexture2D vkInputAttachment = inputAttachment.cast<Texture2D>();
        attachments.add(vkInputAttachment->getView());
    }
    for(auto colorAttachment : layout->colorAttachments)
    {
        PTexture2D vkColorAttachment = colorAttachment.cast<Texture2D>();
        attachments.add(vkColorAttachment->getView());
    }
    if(layout->depthAttachment != nullptr)
    {
        PTexture2D vkDepthAttachment = layout->depthAttachment.cast<Texture2D>();
        attachments.add(vkDepthAttachment->getView());
    }

    VkFramebufferCreateInfo createInfo =
        init::FramebufferCreateInfo(
            renderPass->getHandle(),
            attachments.size(),
            attachments.data(),
            renderPass->getRenderArea().extent.width,
            renderPass->getRenderArea().extent.height,
            1
        );
    VK_CHECK(vkCreateFramebuffer(graphics->getDevice(), &createInfo, nullptr, &handle));
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(graphics->getDevice(), handle, nullptr);
}