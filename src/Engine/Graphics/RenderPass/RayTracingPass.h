#pragma once
#include "Graphics/Graphics.h"
#include "RenderPass.h"

namespace Seele {
class RayTracingPass : public RenderPass {
  public:
    RayTracingPass(Gfx::PGraphics graphics, PScene scene);
    RayTracingPass(RayTracingPass&& other) = default;
    RayTracingPass& operator=(RayTracingPass&& other) = default;
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    Gfx::ODescriptorLayout paramsLayout;
    Gfx::OPipelineLayout pipelineLayout;
    Gfx::OTexture2D radianceAccumulator;
    Gfx::OTexture2D texture;
    Gfx::PTextureCube skyBox;
    Gfx::OSampler skyBoxSampler;
    Gfx::ORayGenShader rayGen;
    Gfx::OAnyHitShader anyhit;
    Gfx::OMissShader miss;
    Gfx::PRayTracingPipeline pipeline;
    Gfx::OTopLevelAS tlas;
    PScene scene;
};
} // namespace Seele