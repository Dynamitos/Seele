#include "Framebuffer.h"
#include "Enums.h"
#include "Initializer.h"
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
    uint32 sizeX = 0;
    uint32 sizeY = 0;
    for (auto inputAttachment : layout->inputAttachments)
    {
        PTexture2D vkInputAttachment = inputAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkInputAttachment->getView());
        description.inputAttachments[description.numInputAttachments++] = vkInputAttachment->getView();
        sizeX = std::max(sizeX, vkInputAttachment->getWidth());
        sizeY = std::max(sizeY, vkInputAttachment->getHeight());
    }
    for (auto colorAttachment : layout->colorAttachments)
    {
        PTexture2D vkColorAttachment = colorAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkColorAttachment->getView());
        description.colorAttachments[description.numColorAttachments++] = vkColorAttachment->getView();
        sizeX = std::max(sizeX, vkColorAttachment->getWidth());
        sizeY = std::max(sizeY, vkColorAttachment->getHeight());
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D vkDepthAttachment = layout->depthAttachment->getTexture().cast<Texture2D>();
        attachments.add(vkDepthAttachment->getView());
        description.depthAttachment = vkDepthAttachment->getView();
        sizeX = std::max(sizeX, vkDepthAttachment->getWidth());
        sizeY = std::max(sizeY, vkDepthAttachment->getHeight());
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

    hash = CRC::Calculate(&description, sizeof(FramebufferDescription), CRC::CRC_32());
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(graphics->getDevice(), handle, nullptr);
    graphics = nullptr;
}