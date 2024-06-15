#pragma once
#include "RenderPass.h"
#include "Graphics/Query.h"

namespace Seele {
class CachedDepthPass : public RenderPass {
  public:
    CachedDepthPass(Gfx::PGraphics graphics, PScene scene);
    CachedDepthPass(CachedDepthPass&&) = default;
    CachedDepthPass& operator=(CachedDepthPass&&) = default;
    virtual ~CachedDepthPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::OTexture2D depthBuffer;
    Gfx::OTexture2D visibilityBuffer;
    Gfx::OPipelineLayout depthPrepassLayout;
    Gfx::OPipelineStatisticsQuery query;

    Gfx::PShaderBuffer cullingBuffer;
};
DEFINE_REF(CachedDepthPass)
} // namespace Seele