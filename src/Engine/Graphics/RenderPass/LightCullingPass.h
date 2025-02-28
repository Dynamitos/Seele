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

    Gfx::OShaderBuffer frustumBuffer;
    constexpr static std::string FRUSTUMBUFFER_NAME = "frustums";
    Gfx::ODescriptorLayout dispatchParamsLayout;
    Gfx::PDescriptorSet dispatchParamsSet;
    Gfx::OComputeShader frustumShader;
    Gfx::PComputePipeline frustumPipeline;
    Gfx::OPipelineLayout frustumLayout;

    PLightEnvironment lightEnv;
    Gfx::PTexture2D depthAttachment;
    constexpr static std::string DEPTHATTACHMENT_NAME = "depth";
    Gfx::OShaderBuffer oLightIndexCounter;
    constexpr static std::string OLIGHTINDEXCOUNTER_NAME = "oLightIndexCounter";
    Gfx::OShaderBuffer tLightIndexCounter;
    constexpr static std::string TLIGHTINDEXCOUNTER_NAME = "tLightIndexCounter";
    Gfx::OShaderBuffer oLightIndexList;
    constexpr static std::string OLIGHTINDEXLIST_NAME = "oLightIndexList";
    Gfx::OShaderBuffer tLightIndexList;
    constexpr static std::string TLIGHTINDEXLIST_NAME = "tLightIndexList";
    Gfx::OTexture2D oLightGrid;
    constexpr static std::string OLIGHTGRID_NAME = "oLightGrid";
    Gfx::OTexture2D tLightGrid;
    constexpr static std::string TLIGHTGRID_NAME = "tLightGrid";
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

    PScene scene;
};
DEFINE_REF(LightCullingPass)
} // namespace Seele
