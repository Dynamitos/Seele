#pragma once
#include "Graphics/RenderTarget.h"
#include "Resources.h"

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics)
class RenderPass : public Gfx::RenderPass {
  public:
    RenderPass(PGraphics graphics, Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport viewport,
               const std::string& name = "");
    virtual ~RenderPass();
    void updateRenderPass();
    MTL::RenderPassDescriptor* getDescriptor() const { return renderPass; }
    const std::string& getName() const { return name; }

  private:
    PGraphics graphics;
    Gfx::PViewport viewport;
    MTL::RenderPassDescriptor* renderPass;
    std::string name;
};
DEFINE_REF(RenderPass)
} // namespace Metal
} // namespace Seele
