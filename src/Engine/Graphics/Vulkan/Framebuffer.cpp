#include "Framebuffer.h"
#include "Enums.h"
#include "RenderPass.h"
#include "Graphics.h"
#include "Texture.h"

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
    uint32 width = 0;
    uint32 height = 0;
    for (auto inputAttachment : layout->inputAttachments)
    {
        PTexture2D vkInputAttachment = inputAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkInputAttachment->getView());
        description.inputAttachments[description.numInputAttachments++] = vkInputAttachment->getView();
        width = std::max(width, vkInputAttachment->getWidth());
        height = std::max(height, vkInputAttachment->getHeight());
    }
    for (auto colorAttachment : layout->colorAttachments)
    {
        PTexture2D vkColorAttachment = colorAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkColorAttachment->getView());
        description.colorAttachments[description.numColorAttachments++] = vkColorAttachment->getView();
        width = std::max(width, vkColorAttachment->getWidth());
        height = std::max(height, vkColorAttachment->getHeight());
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D vkDepthAttachment = layout->depthAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkDepthAttachment->getView());
        description.depthAttachment = vkDepthAttachment->getView();
        width = std::max(width, vkDepthAttachment->getWidth());
        height = std::max(height, vkDepthAttachment->getHeight());
    }
    VkFramebufferCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderPass = renderPass->getHandle(),
        .attachmentCount = (uint32)attachments.size(),
        .pAttachments = attachments.data(),
        .width = width,
        .height = height,
        .layers = 1,
    };
    VK_CHECK(vkCreateFramebuffer(graphics->getDevice(), &createInfo, nullptr, &handle));

    hash = CRC::Calculate(&description, sizeof(FramebufferDescription), CRC::CRC_32());
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(graphics->getDevice(), handle, nullptr);
    graphics = nullptr;
}