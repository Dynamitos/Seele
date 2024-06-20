#pragma once
#include "MinimalEngine.h"
#include "RenderPass.h"

namespace Seele {
DECLARE_REF(CameraActor)
class BasePass : public RenderPass {
  public:
    BasePass(Gfx::PGraphics graphics, PScene scene);
    BasePass(BasePass&&) = default;
    BasePass& operator=(BasePass&&) = default;
    virtual ~BasePass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    Gfx::RenderTargetAttachment colorAttachment;
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::PShaderBuffer oLightIndexList;
    Gfx::PShaderBuffer tLightIndexList;
    Gfx::PTexture2D oLightGrid;
    Gfx::PTexture2D tLightGrid;

    Gfx::PDescriptorSet opaqueCulling;
    Gfx::PDescriptorSet transparentCulling;

    // use a different texture here so we can do multisampling
    Gfx::OTexture2D basePassDepth;

    // used for transparency sorting
    Vector cameraPos;
    Vector cameraForward;

    PCameraActor source;
    Gfx::OPipelineLayout basePassLayout;
    Gfx::ODescriptorLayout lightCullingLayout;

    Gfx::OPipelineStatisticsQuery query;

    Gfx::PShaderBuffer cullingBuffer;
};
DEFINE_REF(BasePass)
} // namespace Seele
