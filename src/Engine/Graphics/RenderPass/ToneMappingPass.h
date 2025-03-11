#pragma once
#include "RenderPass.h"
#include "Graphics/Shader.h"

namespace Seele 
{
class ToneMappingPass : public RenderPass {
  public:
    ToneMappingPass(Gfx::PGraphics graphics);
    ToneMappingPass(ToneMappingPass&&) = default;
    ToneMappingPass& operator=(ToneMappingPass&&) = default;
    virtual ~ToneMappingPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    // non-hdr swapchain output
    Gfx::RenderTargetAttachment colorAttachment;

    Gfx::RenderTargetAttachment hdrInputTexture;
    Gfx::OSampler sampler;

    Gfx::ODescriptorLayout layout;
    Gfx::PDescriptorSet set;
    Gfx::OPipelineLayout pipelineLayout;
    Gfx::OVertexShader vert;
    Gfx::OFragmentShader frag;
    Gfx::PGraphicsPipeline pipeline;
};
}