#include "Framebuffer.h"
#include "CRC.h"
#include "Enums.h"
#include "Graphics.h"
#include "RenderPass.h"
#include "Texture.h"


using namespace Seele;
using namespace Seele::Vulkan;

Framebuffer::Framebuffer(PGraphics graphics, PRenderPass renderPass, Gfx::RenderTargetLayout renderTargetLayout)
    : graphics(graphics), layout(renderTargetLayout), renderPass(renderPass) {
    FramebufferDescription description;
    Array<VkImageView> attachments;
    uint32 width = 0;
    uint32 height = 0;
    uint32 layers = 0;
    for (auto inputAttachment : layout.inputAttachments) {
        PTextureBase vkInputAttachment = inputAttachment.getTexture().cast<TextureBase>();
        attachments.add(vkInputAttachment->getView());
        description.inputAttachments[description.numInputAttachments++] = vkInputAttachment->getView();
        width = std::max(width, vkInputAttachment->getWidth());
        height = std::max(height, vkInputAttachment->getHeight());
        layers = std::max(layers, vkInputAttachment->getNumLayers());
    }
    for (auto colorAttachment : layout.colorAttachments) {
        PTextureBase vkColorAttachment = colorAttachment.getTexture().cast<TextureBase>();
        attachments.add(vkColorAttachment->getView());
        description.colorAttachments[description.numColorAttachments++] = vkColorAttachment->getView();
        width = std::max(width, vkColorAttachment->getWidth());
        height = std::max(height, vkColorAttachment->getHeight());
        layers = std::max(layers, vkColorAttachment->getNumLayers());
    }
    for (auto resolveAttachment : layout.resolveAttachments) {
        PTextureBase vkResolveAttachment = resolveAttachment.getTexture().cast<TextureBase>();
        attachments.add(vkResolveAttachment->getView());
        description.resolveAttachments[description.numResolveAttachments++] = vkResolveAttachment->getView();
        width = std::max(width, vkResolveAttachment->getWidth());
        height = std::max(height, vkResolveAttachment->getHeight());
        layers = std::max(layers, vkResolveAttachment->getNumLayers());
    }
    if (layout.depthAttachment.getTexture() != nullptr) {
        PTextureBase vkDepthAttachment = layout.depthAttachment.getTexture().cast<TextureBase>();
        attachments.add(vkDepthAttachment->getView());
        description.depthAttachment = vkDepthAttachment->getView();
        width = std::max(width, vkDepthAttachment->getWidth());
        height = std::max(height, vkDepthAttachment->getHeight());
        layers = std::max(layers, vkDepthAttachment->getNumLayers());
    }
    if (layout.depthResolveAttachment.getTexture() != nullptr) {
        PTextureBase vkDepthAttachment = layout.depthResolveAttachment.getTexture().cast<TextureBase>();
        attachments.add(vkDepthAttachment->getView());
        description.depthResolveAttachment = vkDepthAttachment->getView();
        width = std::max(width, vkDepthAttachment->getWidth());
        height = std::max(height, vkDepthAttachment->getHeight());
        layers = std::max(layers, vkDepthAttachment->getNumLayers());
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
    renderArea = VkRect2D{
        .offset = {0, 0},
        .extent = {width, height},
    };
    hash = CRC::Calculate(&description, sizeof(FramebufferDescription), CRC::CRC_32());
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(graphics->getDevice(), handle, nullptr);
    graphics = nullptr;
}