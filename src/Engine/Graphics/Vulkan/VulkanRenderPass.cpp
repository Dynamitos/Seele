#include "VulkanRenderPass.h"
#include "VulkanInitializer.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanGraphics.h"
#include "VulkanFramebuffer.h"

using namespace Seele;
using namespace Seele::Vulkan;

RenderPass::RenderPass(PGraphics graphics, Gfx::PRenderTargetLayout layout, Gfx::PViewport viewport)
    : Gfx::RenderPass(layout)
    , graphics(graphics)
{
    renderArea.extent.width = viewport->getSizeX();
    renderArea.extent.height = viewport->getSizeY();
    renderArea.offset.x = viewport->getOffsetX();
    renderArea.offset.y = viewport->getOffsetY();
    subpassContents = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
    Array<VkAttachmentDescription> attachments;
    Array<VkAttachmentReference> inputRefs;
    Array<VkAttachmentReference> colorRefs;
    VkAttachmentReference depthRef;
    uint32 attachmentCounter = 0;
    for (auto inputAttachment : layout->inputAttachments)
    {
        PTexture2D image = inputAttachment->getTexture().cast<Texture2D>();
        VkAttachmentDescription& desc = attachments.add();
        desc.flags = 0;
        desc.format = cast(image->getFormat());
        desc.storeOp = cast(inputAttachment->getStoreOp());
        desc.loadOp = cast(inputAttachment->getLoadOp());
        desc.stencilStoreOp = cast(inputAttachment->getStencilStoreOp());
        desc.stencilLoadOp = cast(inputAttachment->getStencilLoadOp());
        desc.initialLayout = image->isDepthStencil() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        desc.finalLayout = desc.initialLayout;

        VkAttachmentReference &ref = inputRefs.add();
        ref.layout = desc.initialLayout;
        ref.attachment = attachmentCounter;
        attachmentCounter++;
    }
    for (auto colorAttachment : layout->colorAttachments)
    {
        VkAttachmentDescription& desc = attachments.add();
        desc.flags = 0;
        desc.samples = (VkSampleCountFlagBits)colorAttachment->getNumSamples();
        desc.format = cast(colorAttachment->getFormat());
        desc.storeOp = cast(colorAttachment->getStoreOp());
        desc.loadOp = cast(colorAttachment->getLoadOp());
        desc.stencilStoreOp = cast(colorAttachment->getStencilStoreOp());
        desc.stencilLoadOp = cast(colorAttachment->getStencilLoadOp());
        desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkClearValue& clearValue = clearValues.add();
        if(desc.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValue = cast(colorAttachment->clear);
        }

        VkAttachmentReference &ref = colorRefs.add();
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ref.attachment = attachmentCounter;
        attachmentCounter++;
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D image = layout->depthAttachment->getTexture().cast<Texture2D>();
        VkAttachmentDescription& desc = attachments.add();
        desc.flags = 0;
        desc.samples = (VkSampleCountFlagBits)image->getNumSamples();
        desc.format = cast(image->getFormat());
        desc.storeOp = cast(layout->depthAttachment->getStoreOp());
        desc.loadOp = cast(layout->depthAttachment->getLoadOp());
        desc.stencilStoreOp = cast(layout->depthAttachment->getStencilStoreOp());
        desc.stencilLoadOp = cast(layout->depthAttachment->getStencilLoadOp());
        desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkClearValue& clearValue = clearValues.add();
        if(desc.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
        {
            clearValue = cast(layout->depthAttachment->clear);
        }

        VkAttachmentReference &ref = depthRef;
        ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        ref.attachment = attachmentCounter;
        attachmentCounter++;
    }
    VkSubpassDescription subPassDesc =
        init::SubpassDescription(
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            (uint32)colorRefs.size(),
            colorRefs.data(),
            &depthRef,
            (uint32)inputRefs.size(),
            inputRefs.data());
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info =
        init::RenderPassCreateInfo(
            (uint32)attachments.size(),
            attachments.data(),
            1,
            &subPassDesc,
            1,
            &dependency);

    VK_CHECK(vkCreateRenderPass(graphics->getDevice(), &info, nullptr, &renderPass));
}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(graphics->getDevice(), renderPass, nullptr);
}

uint32 RenderPass::getFramebufferHash()
{
    FramebufferDescription description;
    std::memset(&description, 0, sizeof(FramebufferDescription));
    for (auto inputAttachment : layout->inputAttachments)
    {
        PTexture2D tex = inputAttachment->getTexture().cast<Texture2D>();
        description.inputAttachments[description.numInputAttachments++] = tex->getView();
    }
    for (auto colorAttachment : layout->colorAttachments)
    {
        PTexture2D tex = colorAttachment->getTexture().cast<Texture2D>();
        description.colorAttachments[description.numColorAttachments++] = tex->getView();
    }
    if (layout->depthAttachment != nullptr)
    {
        PTexture2D tex = layout->depthAttachment->getTexture().cast<Texture2D>();
        description.depthAttachment = tex->getView();
    }
    boost::crc_32_type result;
    result.process_bytes(&description, sizeof(FramebufferDescription));
    return result.checksum();
}