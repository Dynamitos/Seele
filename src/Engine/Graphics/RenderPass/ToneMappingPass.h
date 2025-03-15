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
    Gfx::OShaderBuffer histogramBuffer;
    Gfx::OShaderBuffer luminanceBuffer;

    float minLogLum = 1;
    float inverseLogLumRange = 1;
    float timeCoeff = 1;
    uint32 numPixels;
    UVector2 threadGroups;
    Gfx::ODescriptorLayout histogramLayout;
    Gfx::PDescriptorSet histogramSet;
    Gfx::OPipelineLayout histogramPipelineLayout;
    Gfx::OComputeShader histogramShader;
    Gfx::PComputePipeline histogramPipeline;
    Gfx::OComputeShader exposureShader;
    Gfx::PComputePipeline exposurePipeline;

    Gfx::RenderTargetAttachment hdrInputTexture;
    Gfx::OSampler sampler;

    Vector4 offset = Vector4(0.0);
    Vector4 slope = Vector4(1.0);
    Vector4 power = Vector4(1.0);
    float sat = 1.0;
    Gfx::ODescriptorLayout tonemappingLayout;
    Gfx::OPipelineLayout tonemappingPipelineLayout;
    Gfx::OVertexShader vert;
    Gfx::OFragmentShader frag;
    Gfx::PGraphicsPipeline pipeline;
};
}