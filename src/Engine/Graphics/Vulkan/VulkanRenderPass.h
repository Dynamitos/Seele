#include "VulkanGraphicsResources.h"

namespace Seele
{
namespace Vulkan
{
class RenderPass : public Gfx::RenderPass
{
public:
    RenderPass(PGraphics graphics, Gfx::PRenderTargetLayout layout);
    virtual ~RenderPass();
    uint32 getFramebufferHash();
    inline VkRenderPass getHandle() const
    {
        return renderPass;
    }
    inline uint32 getClearValueCount() const
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
    inline Gfx::PRenderTargetLayout getLayout()
    {
        return layout;
    }
private:
    PGraphics graphics;
    Gfx::PRenderTargetLayout layout;
    VkRenderPass renderPass;
    Array<VkClearValue> clearValues;
    VkRect2D renderArea;
    VkSubpassContents subpassContents;
};
DEFINE_REF(RenderPass);
} // namespace Vulkan
} // namespace Seele