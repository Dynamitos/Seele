#pragma once
#include "Graphics/RenderTarget.h"
#include "Graphics.h"

namespace Seele
{
namespace Vulkan
{
class RenderPass : public Gfx::RenderPass
{
public:
    RenderPass(PGraphics graphics, Gfx::ORenderTargetLayout layout, Gfx::PViewport viewport);
    virtual ~RenderPass();
    uint32 getFramebufferHash();
    constexpr VkRenderPass getHandle() const
    {
        return renderPass;
    }
    constexpr size_t getClearValueCount() const
    {
        return clearValues.size();
    }
    constexpr VkClearValue *getClearValues() const
    {
        return clearValues.data();
    }
    constexpr VkRect2D getRenderArea() const
    {
        return renderArea;
    }
    constexpr VkSubpassContents getSubpassContents() const
    {
        return subpassContents;
    }
private:
    PGraphics graphics;
    VkRenderPass renderPass;
    Array<VkClearValue> clearValues;
    VkRect2D renderArea;
    VkSubpassContents subpassContents;
};
DEFINE_REF(RenderPass)
} // namespace Vulkan
} // namespace Seele