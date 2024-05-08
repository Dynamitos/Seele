#pragma once
#include "RenderPass.h"

namespace Seele
{
class StaticDepthPrepass : public RenderPass
{
public:
    StaticDepthPrepass(Gfx::PGraphics graphics, PScene scene);
    StaticDepthPrepass(StaticDepthPrepass&&) = default;
    StaticDepthPrepass& operator=(StaticDepthPrepass&&) = default;
    virtual ~StaticDepthPrepass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::OTexture2D depthBuffer;
    Gfx::OPipelineLayout depthPrepassLayout;
    Gfx::ODescriptorLayout sceneDataLayout;
};
}