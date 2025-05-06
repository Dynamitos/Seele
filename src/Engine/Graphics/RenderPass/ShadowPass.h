#pragma once
#include "RenderPass.h"

namespace Seele {
class ShadowPass : public RenderPass {
  public:
    ShadowPass(Gfx::PGraphics graphics, PScene scene);
    ShadowPass(ShadowPass&& other) = default;
    ShadowPass& operator=(ShadowPass&& other) = default;
    virtual ~ShadowPass();
    virtual void beginFrame(const Component::Camera& cam, const Component::Transform& transform) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
  private:
    AABB cameraFrustumBox;
    Gfx::OPipelineLayout shadowLayout;
    Gfx::PShaderBuffer cullingBuffer;
    Gfx::OViewport shadowViewport;
    PScene scene;
};
DEFINE_REF(ShadowPass)
} // namespace Seele