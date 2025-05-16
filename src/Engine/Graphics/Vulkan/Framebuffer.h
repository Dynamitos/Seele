#pragma once
#include "Enums.h"
#include "Graphics.h"
#include "Graphics/RenderTarget.h"

namespace Seele {
namespace Vulkan {
DECLARE_REF(RenderPass)
struct FramebufferDescription {
    StaticArray<VkImageView, 16> inputAttachments;
    StaticArray<VkImageView, 16> colorAttachments;
    StaticArray<VkImageView, 16> resolveAttachments;
    VkImageView depthAttachment = VK_NULL_HANDLE;
    VkImageView depthResolveAttachment = VK_NULL_HANDLE;
    uint32 numInputAttachments = 0;
    uint32 numColorAttachments = 0;
    uint32 numResolveAttachments = 0;
};
class Framebuffer {
  public:
    Framebuffer(PGraphics graphics, PRenderPass renderpass, Gfx::RenderTargetLayout renderTargetLayout);
    virtual ~Framebuffer();
    constexpr VkFramebuffer getHandle() const { return handle; }
    constexpr uint32 getHash() const { return hash; }
    constexpr VkRect2D getRenderArea() const { return renderArea; }

  private:
    uint32 hash;
    VkRect2D renderArea;
    PGraphics graphics;
    VkFramebuffer handle;
    Gfx::RenderTargetLayout layout;
    PRenderPass renderPass;
};
DEFINE_REF(Framebuffer)
} // namespace Vulkan
} // namespace Seele