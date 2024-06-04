#pragma once
#include "RenderPass.h"

namespace Seele
{
class VisibilityPass : public RenderPass
{
public:
    VisibilityPass(Gfx::PGraphics graphics, PScene scene);
    VisibilityPass(VisibilityPass&&) = default;
    VisibilityPass& operator=(VisibilityPass&&) = default;
    virtual ~VisibilityPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
    uint64 allocate
private:
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::PDescriptorSet visibilitySet;
    Gfx::ODescriptorLayout visibilityDescriptor;
    Gfx::OPipelineLayout visibilityLayout;
    Gfx::PComputePipeline visibilityPipeline;

    // Holds culling information for every meshlet for each instance
    Gfx::OShaderBuffer cullingBuffer;
};
DEFINE_REF(VisibilityPass)
}