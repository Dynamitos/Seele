#pragma once
#include "Graphics.h"
#include "Resources.h"
#include "Graphics/RenderTarget.h"

namespace Seele {
namespace Vulkan {

class RenderPass : public Gfx::RenderPass {
  public:
    RenderPass(PGraphics graphics, Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, std::string name,
               Array<uint32> viewMasks = {}, Array<uint32> correlationMasks = {});
    virtual ~RenderPass();
    uint32 getFramebufferHash();
    void endRenderPass();
    constexpr VkRenderPass getHandle() const { return renderPass->getHandle(); }
    constexpr size_t getClearValueCount() const { return clearValues.size(); }
    constexpr VkClearValue* getClearValues() const { return clearValues.data(); }
    constexpr VkSubpassContents getSubpassContents() const { return subpassContents; }
    constexpr const std::string& getName() const { return name; }
    constexpr PRenderPassHandle getCommandHandle() const { return renderPass; }

  private:
    PGraphics graphics;
    std::string name;
    ORenderPassHandle renderPass;
    Array<VkClearValue> clearValues;
    VkRect2D renderArea;
    VkSubpassContents subpassContents;
};
DEFINE_REF(RenderPass)
} // namespace Vulkan
} // namespace Seele