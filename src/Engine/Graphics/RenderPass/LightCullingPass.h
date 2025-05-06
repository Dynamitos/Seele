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
    virtual void beginFrame(const Component::Camera& cam, const Component::Transform& transform) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    void setupFrustums();
    static constexpr uint32 BLOCK_SIZE = 32;
    static constexpr uint32 INDEX_LIGHT_ENV = 1;

    Gfx::OShaderBuffer frustumBuffer;
    const char* FRUSTUMBUFFER_NAME = "frustums";
    Gfx::ODescriptorLayout dispatchParamsLayout;
    Gfx::PDescriptorSet dispatchParamsSet;
    Gfx::OComputeShader frustumShader;
    Gfx::PComputePipeline frustumPipeline;
    Gfx::OPipelineLayout frustumLayout;
    Gfx::PDescriptorSet viewParamsSet;

    PLightEnvironment lightEnv;
    Gfx::PTexture2D depthAttachment;
    constexpr static const char* DEPTHATTACHMENT_NAME = "depth";
    Gfx::OShaderBuffer oLightIndexCounter;
    constexpr static const char* OLIGHTINDEXCOUNTER_NAME = "oLightIndexCounter";
    Gfx::OShaderBuffer tLightIndexCounter;
    constexpr static const char* TLIGHTINDEXCOUNTER_NAME = "tLightIndexCounter";
    Gfx::OShaderBuffer oLightIndexList;
    constexpr static const char* OLIGHTINDEXLIST_NAME = "oLightIndexList";
    Gfx::OShaderBuffer tLightIndexList;
    constexpr static const char* TLIGHTINDEXLIST_NAME = "tLightIndexList";
    Gfx::OTexture2D oLightGrid;
    constexpr static const char* OLIGHTGRID_NAME = "oLightGrid";
    Gfx::OTexture2D tLightGrid;
    constexpr static const char* TLIGHTGRID_NAME = "tLightGrid";
    Gfx::PDescriptorSet cullingDescriptorSet;
    Gfx::ODescriptorLayout cullingDescriptorLayout;
    Gfx::OPipelineLayout cullingLayout;
    Gfx::OPipelineLayout cullingEnableLayout;
    Gfx::OComputeShader cullingShader;
    Gfx::OComputeShader cullingEnabledShader;
    Gfx::PComputePipeline cullingPipeline;
    Gfx::PComputePipeline cullingEnabledPipeline;
    Gfx::OPipelineStatisticsQuery query;
    Gfx::PTimestampQuery timestamps;
    UVector4 numThreadGroups;
    UVector4 numThreads;
    
    PScene scene;
};
DEFINE_REF(LightCullingPass)
} // namespace Seele
