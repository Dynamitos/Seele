#include "RenderPass.h"
#include "Graphics.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "RenderPass.h"

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
    Array<VkAttachmentDescription> attachments;
    Array<VkAttachmentReference> inputRefs;
    Array<VkAttachmentReference> colorRefs;
    VkAttachmentReference depthRef;
    uint32 attachmentCounter = 0;
    for (auto& inputAttachment : layout->inputAttachments)
    {
        PTexture2D image = inputAttachment->getTexture().cast<Texture2D>();
        VkAttachmentDescription& desc = attachments.add() = {
            .flags = 0,
            .format = cast(image->getFormat()),
            .loadOp = cast(inputAttachment->getLoadOp()),
            .storeOp = cast(inputAttachment->getStoreOp()),
            .stencilLoadOp = cast(inputAttachment->getStencilLoadOp()),
            .stencilStoreOp = cast(inputAttachment->getStencilStoreOp()),
            .initialLayout = image->isDepthStencil() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = desc.initialLayout,
        };
        
        inputRefs.add() = {
            .attachment = attachmentCounter++,
            .layout = desc.initialLayout,
        };
    }
    for (auto& colorAttachment : layout->colorAttachments)
    {
        attachments.add() = {
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
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D image = layout->depthAttachment->getTexture().cast<Texture2D>();
        attachments.add() = {
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
        if(attachments.back().loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValue = cast(layout->depthAttachment->clear);
        }

        depthRef = {
            .attachment = attachmentCounter++,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
    }
    VkSubpassDescription subPassDesc = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = (uint32)inputRefs.size(),
        .pInputAttachments = inputRefs.data(),
        .colorAttachmentCount = (uint32)colorRefs.size(),
        .pColorAttachments = colorRefs.data(),
        .pDepthStencilAttachment = &depthRef,
    };
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    
    VkRenderPassCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .attachmentCount = (uint32)attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subPassDesc,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VK_CHECK(vkCreateRenderPass(graphics->getDevice(), &info, nullptr, &renderPass));

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
    framebufferHash = CRC::Calculate(&description, sizeof(FramebufferDescription), CRC::CRC_32());
}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(graphics->getDevice(), renderPass, nullptr);
}

