#pragma once
#include "Graphics/Graphics.h"
#include "RenderPass.h"

namespace Seele {
class RayTracingPass : public RenderPass {
  public:
    RayTracingPass(Gfx::PGraphics graphics, PScene scene);
    RayTracingPass(RayTracingPass&& other) = default;
    RayTracingPass& operator=(RayTracingPass&& other) = default;
    virtual void beginFrame(const Component::Camera& cam, const Component::Transform& transform) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    Gfx::ODescriptorLayout paramsLayout;
    Gfx::OPipelineLayout pipelineLayout;
    constexpr static const char* TLAS_NAME = "scene";
    Gfx::OTopLevelAS tlas;
    constexpr static const char* ACCUMULATOR_NAME = "accumulator";
    Gfx::OTexture2D radianceAccumulator;
    constexpr static const char* TEXTURE_NAME = "image";
    Gfx::OTexture2D texture;
    constexpr static const char* SKYBOX_NAME = "skybox";
    Gfx::PTextureCube skyBox;
    constexpr static const char* SKYSAMPLER_NAME = "sampler";
    Gfx::OSampler skyBoxSampler;
    constexpr static const char* INDEXBUFFER_NAME = "indexBuffer";
    Gfx::ORayGenShader rayGen;
    Gfx::OAnyHitShader anyhit;
    Gfx::OMissShader miss;
    Gfx::PRayTracingPipeline pipeline;
    Gfx::ODescriptorSet viewParamsSet;
    PScene scene;
};
} // namespace Seele