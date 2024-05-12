#pragma once
#include "MinimalEngine.h"
#include "RenderPass.h"

namespace Seele
{
class DepthPrepass : public RenderPass
{
public:
    DepthPrepass(Gfx::PGraphics graphics, PScene scene);
    DepthPrepass(DepthPrepass&&) = default;
    DepthPrepass& operator=(DepthPrepass&&) = default;
    virtual ~DepthPrepass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::RenderTargetAttachment depthAttachment;

    Gfx::OTexture2D depthBuffer;

    Gfx::OPipelineLayout depthPrepassLayout;
};
DEFINE_REF(DepthPrepass)
} // namespace Seele
