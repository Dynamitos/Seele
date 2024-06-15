#pragma once
#include "Graphics/Query.h"
#include "Graphics/Shader.h"
#include "RenderPass.h"
#include "Scene/Scene.h"


namespace Seele {
DECLARE_REF(CameraActor)
DECLARE_REF(Scene)
DECLARE_REF(Viewport)
class LightCullingPass : public RenderPass {
  public:
    LightCullingPass(Gfx::PGraphics graphics, PScene scene);
    LightCullingPass(LightCullingPass&&) = default;
    LightCullingPass& operator=(LightCullingPass&&) = default;
    virtual ~LightCullingPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    void setupFrustums();
    static constexpr uint32 BLOCK_SIZE = 32;
    static constexpr uint32 INDEX_LIGHT_ENV = 1;
    struct DispatchParams {
        glm::uvec3 numThreadGroups;
        uint32_t pad0;
        glm::uvec3 numThreads;
        uint32_t pad1;
    } dispatchParams;
    struct Plane {
        Vector n;
        float d;
    };
    struct Frustum {
        Plane planes[4];
    };

    Gfx::OShaderBuffer frustumBuffer;
    Gfx::OUniformBuffer dispatchParamsBuffer;
    Gfx::OUniformBuffer viewParamsBuffer;
    Gfx::ODescriptorLayout dispatchParamsLayout;
    Gfx::PDescriptorSet dispatchParamsSet;
    Gfx::OComputeShader frustumShader;
    Gfx::PComputePipeline frustumPipeline;
    Gfx::OPipelineLayout frustumLayout;

    PLightEnvironment lightEnv;
    Gfx::PTexture2D depthAttachment;
    Gfx::OShaderBuffer oLightIndexCounter;
    Gfx::OShaderBuffer tLightIndexCounter;
    Gfx::OShaderBuffer oLightIndexList;
    Gfx::OShaderBuffer tLightIndexList;
    Gfx::OTexture2D oLightGrid;
    Gfx::OTexture2D tLightGrid;
    Gfx::PDescriptorSet cullingDescriptorSet;
    Gfx::ODescriptorLayout cullingDescriptorLayout;
    Gfx::OPipelineLayout cullingLayout;
    Gfx::OPipelineLayout cullingEnableLayout;
    Gfx::OComputeShader cullingShader;
    Gfx::OComputeShader cullingEnabledShader;
    Gfx::PComputePipeline cullingPipeline;
    Gfx::PComputePipeline cullingEnabledPipeline;
    Gfx::OPipelineStatisticsQuery query;
};
DEFINE_REF(LightCullingPass)
} // namespace Seele
