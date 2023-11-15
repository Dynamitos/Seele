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
    VkImageView depthAttachment;
    uint32 numInputAttachments;
    uint32 numColorAttachments;
};
class Framebuffer
{
public:
    Framebuffer(PGraphics graphics, PRenderPass renderpass, Gfx::PRenderTargetLayout renderTargetLayout);
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
    Gfx::PRenderTargetLayout layout;
    PRenderPass renderPass;
};
DEFINE_REF(Framebuffer)
} // namespace Vulkan
} // namespace Seele