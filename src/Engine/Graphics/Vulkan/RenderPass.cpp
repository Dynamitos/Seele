#include "RenderPass.h"
#include "Graphics.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "Resources.h"
#include "Command.h"

using namespace Seele;
using namespace Seele::Vulkan;

RenderPass::RenderPass(PGraphics graphics, Gfx::ORenderTargetLayout _layout, Gfx::PViewport viewport)
    : Gfx::RenderPass(std::move(_layout))
    , graphics(graphics)
{
    renderArea.extent.width = viewport->getWidth();
    renderArea.extent.height = viewport->getHeight();
    renderArea.offset.x = viewport->getOffsetX();
    renderArea.offset.y = viewport->getOffsetY();
    subpassContents = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
    Array<VkAttachmentDescription2> attachments;
    Array<VkAttachmentReference2> inputRefs;
    Array<VkAttachmentReference2> colorRefs;
    Array<VkAttachmentReference2> resolveRefs;
    VkAttachmentReference2 depthRef;
    VkAttachmentReference2 depthResolveRef;

    uint32 attachmentCounter = 0;
    for (auto& inputAttachment : layout->inputAttachments)
    {
        PTexture2D image = inputAttachment->getTexture().cast<Texture2D>();
        VkAttachmentDescription2& desc = attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(image->getFormat()),
            .loadOp = cast(inputAttachment->getLoadOp()),
            .storeOp = cast(inputAttachment->getStoreOp()),
            .stencilLoadOp = cast(inputAttachment->getStencilLoadOp()),
            .stencilStoreOp = cast(inputAttachment->getStencilStoreOp()),
            .initialLayout = image->isDepthStencil() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = image->isDepthStencil() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
        
        inputRefs.add() = {
            .attachment = attachmentCounter++,
            .layout = desc.initialLayout,
        };
    }
    for (auto& colorAttachment : layout->colorAttachments)
    {
        attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(colorAttachment->getFormat()),
            .samples = (VkSampleCountFlagBits)colorAttachment->getNumSamples(),
            .loadOp = cast(colorAttachment->getLoadOp()),
            .storeOp = cast(colorAttachment->getStoreOp()),
            .stencilLoadOp = cast(colorAttachment->getStencilLoadOp()),
            .stencilStoreOp = cast(colorAttachment->getStencilStoreOp()),
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
        
        VkClearValue& clearValue = clearValues.add();
        if(attachments.back().loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValue = cast(colorAttachment->clear);
        }

        colorRefs.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .pNext = nullptr,
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }
    for (auto& resolveAttachment : layout->resolveAttachments)
    {
        attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(resolveAttachment->getFormat()),
            .samples = (VkSampleCountFlagBits)resolveAttachment->getNumSamples(),
            .loadOp = cast(resolveAttachment->getLoadOp()),
            .storeOp = cast(resolveAttachment->getStoreOp()),
            .stencilLoadOp = cast(resolveAttachment->getStencilLoadOp()),
            .stencilStoreOp = cast(resolveAttachment->getStencilStoreOp()),
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkClearValue& clearValue = clearValues.add();
        if (attachments.back().loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValue = cast(resolveAttachment->clear);
        }

        resolveRefs.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .pNext = nullptr,
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D image = layout->depthAttachment->getTexture().cast<Texture2D>();
        attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(image->getFormat()),
            .samples = (VkSampleCountFlagBits)image->getNumSamples(),
            .loadOp = cast(layout->depthAttachment->getLoadOp()),
            .storeOp = cast(layout->depthAttachment->getStoreOp()),
            .stencilLoadOp = cast(layout->depthAttachment->getStencilLoadOp()),
            .stencilStoreOp = cast(layout->depthAttachment->getStencilStoreOp()),
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        VkClearValue& clearValue = clearValues.add();
        if (attachments.back().loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValue = cast(layout->depthAttachment->clear);
        }

        depthRef = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .pNext = nullptr,
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
    }
    if (layout->depthResolveAttachment != nullptr)
    {
        PTexture2D image = layout->depthResolveAttachment->getTexture().cast<Texture2D>();
        attachments.add() = {
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = 0,
            .format = cast(image->getFormat()),
            .samples = (VkSampleCountFlagBits)image->getNumSamples(),
            .loadOp = cast(layout->depthResolveAttachment->getLoadOp()),
            .storeOp = cast(layout->depthResolveAttachment->getStoreOp()),
            .stencilLoadOp = cast(layout->depthResolveAttachment->getStencilLoadOp()),
            .stencilStoreOp = cast(layout->depthResolveAttachment->getStencilStoreOp()),
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
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
        .pNext = layout->depthResolveAttachment != nullptr ? &depthResolve : nullptr,
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = (uint32)inputRefs.size(),
        .pInputAttachments = inputRefs.data(),
        .colorAttachmentCount = (uint32)colorRefs.size(),
        .pColorAttachments = colorRefs.data(),
        .pResolveAttachments = resolveRefs.size() > 0 ? resolveRefs.data() : nullptr,
        .pDepthStencilAttachment = &depthRef,
    };
    VkSubpassDependency2 dependency = {
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
        .pNext = nullptr,
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    
    VkRenderPassCreateInfo2 info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .pNext = nullptr,
        .attachmentCount = (uint32)attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subPassDesc,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VK_CHECK(vkCreateRenderPass2(graphics->getDevice(), &info, nullptr, &renderPass));
}

RenderPass::~RenderPass()
{
    graphics->getDestructionManager()->queueRenderPass(graphics->getGraphicsCommands()->getCommands(), renderPass);
}

uint32 RenderPass::getFramebufferHash()
{
    FramebufferDescription description;
    std::memset(&description, 0, sizeof(FramebufferDescription));
    for (auto& inputAttachment : layout->inputAttachments)
    {
        PTexture2D tex = inputAttachment->getTexture().cast<Texture2D>();
        description.inputAttachments[description.numInputAttachments++] = tex->getView();
    }
    for (auto& colorAttachment : layout->colorAttachments)
    {
        PTexture2D tex = colorAttachment->getTexture().cast<Texture2D>();
        description.colorAttachments[description.numColorAttachments++] = tex->getView();
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D tex = layout->depthAttachment->getTexture().cast<Texture2D>();
        description.depthAttachment = tex->getView();
    }
    return CRC::Calculate(&description, sizeof(FramebufferDescription), CRC::CRC_32());
}
