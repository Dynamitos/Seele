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
    inline VkRenderPass getHandle() const
    {
        return renderPass;
    }
    inline size_t getClearValueCount() const
    {
        return clearValues.size();
    }
    inline VkClearValue *getClearValues() const
    {
        return clearValues.data();
    }
    inline VkRect2D getRenderArea() const
    {
        return renderArea;
    }
    inline VkSubpassContents getSubpassContents() const
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