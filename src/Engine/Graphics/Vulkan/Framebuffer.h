#pragma once
#include "Enums.h"
#include "Graphics.h"
#include "Graphics/RenderTarget.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(RenderPass)
struct FramebufferDescription
{
    StaticArray<VkImageView, 16> inputAttachments;
    StaticArray<VkImageView, 16> colorAttachments;
    StaticArray<VkImageView, 16> resolveAttachments;
    VkImageView depthAttachment;
    VkImageView depthResolveAttachment;
    uint32 numInputAttachments;
    uint32 numColorAttachments;
    uint32 numResolveAttachments;
};
class Framebuffer
{
public:
    Framebuffer(PGraphics graphics, PRenderPass renderpass, Gfx::RenderTargetLayout renderTargetLayout);
    virtual ~Framebuffer();
    constexpr VkFramebuffer getHandle() const
    {
        return handle;
    }
    constexpr uint32 getHash() const
    {
        return hash;
    }

private:
    uint32 hash;
    PGraphics graphics;
    VkFramebuffer handle;
    Gfx::RenderTargetLayout layout;
    PRenderPass renderPass;
};
DEFINE_REF(Framebuffer)
} // namespace Vulkan
} // namespace Seele