#pragma once
#include "MinimalEngine.h"
#include "RenderPass.h"
#include "Graphics/Query.h"

namespace Seele {
class DepthCullingPass : public RenderPass {
  public:
    DepthCullingPass(Gfx::PGraphics graphics, PScene scene);
    DepthCullingPass(DepthCullingPass&&) = default;
    DepthCullingPass& operator=(DepthCullingPass&&) = default;
    virtual ~DepthCullingPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    Gfx::OTexture2D depthMipTexture;
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::ODescriptorLayout depthTextureLayout;
    Gfx::OPipelineLayout depthPrepassLayout;

    Gfx::PShaderBuffer cullingBuffer;
};
DEFINE_REF(DepthCullingPass)
} // namespace Seele
