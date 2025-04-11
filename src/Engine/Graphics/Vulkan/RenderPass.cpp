#include "RenderPass.h"
#include "CRC.h"
#include "Command.h"
#include "Framebuffer.h"
#include "Graphics.h"
#include "Resources.h"
#include "Texture.h"

using namespace Seele;
using namespace Seele::Vulkan;

RenderPass::RenderPass(PGraphics graphics, Gfx::RenderTargetLayout _layout, Array<Gfx::SubPassDependency> _dependencies,
                       std::string name, Array<uint32> viewMasks, Array<uint32> correlationMasks)
    : Gfx::RenderPass(std::move(_layout), std::move(_dependencies)), graphics(graphics) {
    subpassContents = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
    Array<VkAttachmentDescription2> attachments;
    Array<VkAttachmentReference2> inputRefs;
    Array<VkAttachmentReference2> colorRefs;
    Array<VkAttachmentReference2> resolveRefs;
    VkAttachmentReference2 depthRef;
    VkAttachmentReference2 depthResolveRef;

    uint32 attachmentCounter = 0;
    for (auto& inputAttachment : layout.inputAttachments) {
        PTexture2D image = inputAttachment.getTexture().cast<Texture2D>();
        VkAttachmentDescription2& desc = attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(image->getFormat()),
            .loadOp = cast(inputAttachment.getLoadOp()),
            .storeOp = cast(inputAttachment.getStoreOp()),
            .stencilLoadOp = cast(inputAttachment.getStencilLoadOp()),
            .stencilStoreOp = cast(inputAttachment.getStencilStoreOp()),
            .initialLayout = cast(inputAttachment.getInitialLayout()),
            .finalLayout = cast(inputAttachment.getFinalLayout()),
        };

        inputRefs.add() = {
            .attachment = attachmentCounter++,
            .layout = desc.initialLayout,
        };
    }
    for (auto& colorAttachment : layout.colorAttachments) {
        attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(colorAttachment.getFormat()),
            .samples = (VkSampleCountFlagBits)colorAttachment.getNumSamples(),
            .loadOp = cast(colorAttachment.getLoadOp()),
            .storeOp = cast(colorAttachment.getStoreOp()),
            .stencilLoadOp = cast(colorAttachment.getStencilLoadOp()),
            .stencilStoreOp = cast(colorAttachment.getStencilStoreOp()),
            .initialLayout = cast(colorAttachment.getInitialLayout()),
            .finalLayout = cast(colorAttachment.getFinalLayout()),
        };

        VkClearValue& clearValue = clearValues.add();
        if (attachments.back().loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
            clearValue = cast(colorAttachment.clear);
        }

        colorRefs.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .pNext = nullptr,
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }
    for (auto& resolveAttachment : layout.resolveAttachments) {
        attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(resolveAttachment.getFormat()),
            .samples = (VkSampleCountFlagBits)resolveAttachment.getTexture()->getNumSamples(),
            .loadOp = cast(resolveAttachment.getLoadOp()),
            .storeOp = cast(resolveAttachment.getStoreOp()),
            .stencilLoadOp = cast(resolveAttachment.getStencilLoadOp()),
            .stencilStoreOp = cast(resolveAttachment.getStencilStoreOp()),
            .initialLayout = cast(resolveAttachment.getInitialLayout()),
            .finalLayout = cast(resolveAttachment.getFinalLayout()),
        };

        VkClearValue& clearValue = clearValues.add();
        if (attachments.back().loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
            clearValue = cast(resolveAttachment.clear);
        }

        resolveRefs.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .pNext = nullptr,
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }
    if (layout.depthAttachment.getTexture() != nullptr) {
        PTexture2D image = layout.depthAttachment.getTexture().cast<Texture2D>();
        attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(image->getFormat()),
            .samples = (VkSampleCountFlagBits)image->getNumSamples(),
            .loadOp = cast(layout.depthAttachment.getLoadOp()),
            .storeOp = cast(layout.depthAttachment.getStoreOp()),
            .stencilLoadOp = cast(layout.depthAttachment.getStencilLoadOp()),
            .stencilStoreOp = cast(layout.depthAttachment.getStencilStoreOp()),
            .initialLayout = cast(layout.depthAttachment.getInitialLayout()),
            .finalLayout = cast(layout.depthAttachment.getFinalLayout()),
        };

        VkClearValue& clearValue = clearValues.add();
        if (attachments.back().loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
            clearValue = cast(layout.depthAttachment.clear);
        }

        depthRef = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .pNext = nullptr,
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
    }
    if (layout.depthResolveAttachment.getTexture() != nullptr) {
        PTexture2D image = layout.depthResolveAttachment.getTexture().cast<Texture2D>();
        attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(image->getFormat()),
            .samples = (VkSampleCountFlagBits)image->getNumSamples(),
            .loadOp = cast(layout.depthResolveAttachment.getLoadOp()),
            .storeOp = cast(layout.depthResolveAttachment.getStoreOp()),
            .stencilLoadOp = cast(layout.depthResolveAttachment.getStencilLoadOp()),
            .stencilStoreOp = cast(layout.depthResolveAttachment.getStencilStoreOp()),
            .initialLayout = cast(layout.depthResolveAttachment.getInitialLayout()),
            .finalLayout = cast(layout.depthResolveAttachment.getFinalLayout()),
        };
        VkClearValue& clearValue = clearValues.add();
        if (attachments.back().loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
            clearValue = cast(layout.depthResolveAttachment.clear);
        }
        depthResolveRef = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .pNext = nullptr,
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
    }

    VkSubpassDescriptionDepthStencilResolve depthResolve = {
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE,
        .pNext = nullptr,
        .depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,
        .stencilResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,
        .pDepthStencilResolveAttachment = &depthResolveRef,
    };
    VkSubpassDescription2 subPassDesc = {
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
        .pNext = layout.depthResolveAttachment.getTexture() != nullptr ? &depthResolve : nullptr,
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .viewMask = viewMasks[0],
        .inputAttachmentCount = (uint32)inputRefs.size(),
        .pInputAttachments = inputRefs.data(),
        .colorAttachmentCount = (uint32)colorRefs.size(),
        .pColorAttachments = colorRefs.data(),
        .pResolveAttachments = resolveRefs.size() > 0 ? resolveRefs.data() : nullptr,
        .pDepthStencilAttachment = layout.depthAttachment.getTexture() != nullptr ? &depthRef : nullptr,
    };
    Array<VkSubpassDependency2> dep;
    for (const auto& d : dependencies) {
        dep.add() = {
            .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
            .pNext = nullptr,
            .srcSubpass = d.srcSubpass,
            .dstSubpass = d.dstSubpass,
            .srcStageMask = d.srcStage,
            .dstStageMask = d.dstStage,
            .srcAccessMask = d.srcAccess,
            .dstAccessMask = d.dstAccess,
            .dependencyFlags = 0,
        };
    }
    VkRenderPassCreateInfo2 info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .pNext = nullptr,
        .attachmentCount = (uint32)attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subPassDesc,
        .dependencyCount = (uint32)dep.size(),
        .pDependencies = dep.data(),
        .correlatedViewMaskCount = (uint32)correlationMasks.size(),
        .pCorrelatedViewMasks = correlationMasks.data(),
    };
    VK_CHECK(vkCreateRenderPass2(graphics->getDevice(), &info, nullptr, &renderPass));
    VkDebugUtilsObjectNameInfoEXT nameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_RENDER_PASS,
        .objectHandle = (uint64)renderPass,
        .pObjectName = name.c_str(),
    };
    assert(!name.empty());
    vkSetDebugUtilsObjectNameEXT(graphics->getDevice(), &nameInfo);
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(graphics->getDevice(), renderPass, nullptr);
    // graphics->getDestructionManager()->queueRenderPass(graphics->getGraphicsCommands()->getCommands(), renderPass);
}

uint32 RenderPass::getFramebufferHash() {
    FramebufferDescription description;
    std::memset(&description, 0, sizeof(FramebufferDescription));
    for (auto& inputAttachment : layout.inputAttachments) {
        PTextureBase tex = inputAttachment.getTexture().cast<TextureBase>();
        description.inputAttachments[description.numInputAttachments++] = tex->getView();
    }
    for (auto& colorAttachment : layout.colorAttachments) {
        PTextureBase tex = colorAttachment.getTexture().cast<TextureBase>();
        description.colorAttachments[description.numColorAttachments++] = tex->getView();
    }
    for (auto& resolveAttachment : layout.resolveAttachments) {
        PTextureBase tex = resolveAttachment.getTexture().cast<TextureBase>();
        description.resolveAttachments[description.numResolveAttachments++] = tex->getView();
    }
    if (layout.depthAttachment.getTexture() != nullptr) {
        PTextureBase tex = layout.depthAttachment.getTexture().cast<TextureBase>();
        description.depthAttachment = tex->getView();
    }
    if (layout.depthResolveAttachment.getTexture() != nullptr) {
        PTextureBase tex = layout.depthResolveAttachment.getTexture().cast<TextureBase>();
        description.depthResolveAttachment = tex->getView();
    }
    return CRC::Calculate(&description, sizeof(FramebufferDescription), CRC::CRC_32());
}

void RenderPass::endRenderPass() {
    for (auto& inputAttachment : layout.inputAttachments) {
        PTextureBase tex = inputAttachment.getTexture().cast<TextureBase>();
        tex->setLayout(inputAttachment.getFinalLayout());
    }
    for (auto& colorAttachment : layout.colorAttachments) {
        PTextureBase tex = colorAttachment.getTexture().cast<TextureBase>();
        tex->setLayout(colorAttachment.getFinalLayout());
    }
    for (auto& resolveAttachment : layout.resolveAttachments) {
        PTextureBase tex = resolveAttachment.getTexture().cast<TextureBase>();
        tex->setLayout(resolveAttachment.getFinalLayout());
    }
    if (layout.depthAttachment.getTexture() != nullptr) {
        PTextureBase tex = layout.depthAttachment.getTexture().cast<TextureBase>();
        tex->setLayout(layout.depthAttachment.getFinalLayout());
    }
    if (layout.depthResolveAttachment.getTexture() != nullptr) {
        PTextureBase tex = layout.depthResolveAttachment.getTexture().cast<TextureBase>();
        tex->setLayout(layout.depthResolveAttachment.getFinalLayout());
    }
}
