#pragma once
#include "RenderPass.h"

namespace Seele
{
DECLARE_REF(CameraActor)
class StaticBasePass : public RenderPass
{
public:

    StaticBasePass(Gfx::PGraphics graphics, PScene scene);
    StaticBasePass(StaticBasePass&&) = default;
    StaticBasePass& operator=(StaticBasePass&&) = default;
    virtual ~StaticBasePass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::RenderTargetAttachment colorAttachment;
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::RenderTargetAttachment meshletIdAttachment;
    Gfx::PShaderBuffer oLightIndexList;
    Gfx::PShaderBuffer tLightIndexList;
    Gfx::PTexture2D oLightGrid;
    Gfx::PTexture2D tLightGrid;
    Gfx::OTexture2D meshletIdTexture;

    Gfx::PDescriptorSet opaqueCulling;
    Gfx::PDescriptorSet transparentCulling;

    PCameraActor source;
    Gfx::OPipelineLayout basePassLayout;
    Gfx::ODescriptorLayout lightCullingLayout;
};
}