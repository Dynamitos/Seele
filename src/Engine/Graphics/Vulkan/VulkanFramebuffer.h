#pragma once
#include "VulkanGraphicsResources.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(RenderPass)
struct FramebufferDescription
{
    VkImageView inputAttachments[16];
    VkImageView colorAttachments[16];
    VkImageView depthAttachment;
    uint32 numInputAttachments;
    uint32 numColorAttachments;
};
class Framebuffer
{
public:
    Framebuffer(PGraphics graphics, PRenderPass renderpass, Gfx::PRenderTargetLayout renderTargetLayout);
    virtual ~Framebuffer();
    inline VkFramebuffer getHandle() const
    {
        return handle;
    }
    inline uint32 getHash() const
    {
        return hash;
    }

private:
    uint32 hash;
    PGraphics graphics;
    VkFramebuffer handle;
    Gfx::PRenderTargetLayout layout;
    PRenderPass renderPass;
};
DEFINE_REF(Framebuffer)
} // namespace Vulkan
} // namespace Seele