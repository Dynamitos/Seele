#pragma once
#include "MinimalEngine.h"
#include "RenderPass.h"
#include "WaterRenderer.h"

namespace Seele {
DECLARE_REF(CameraActor)
class BasePass : public RenderPass {
  public:
    BasePass(Gfx::PGraphics graphics, PScene scene);
    BasePass(BasePass&&) = default;
    BasePass& operator=(BasePass&&) = default;
    virtual ~BasePass();
    virtual void beginFrame(const Component::Camera& cam, const Component::Transform& transform) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
      // hdr
    Gfx::RenderTargetAttachment msColorAttachment;
    Gfx::RenderTargetAttachment colorAttachment;
    Gfx::RenderTargetAttachment msDepthAttachment;
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::PShaderBuffer oLightIndexList;
    Gfx::PShaderBuffer tLightIndexList;
    Gfx::PTexture2D oLightGrid;
    Gfx::PTexture2D tLightGrid;
    constexpr static const char* LIGHTINDEX_NAME = "lightIndexList";
    constexpr static const char* LIGHTGRID_NAME = "lightGrid";

    Gfx::PDescriptorSet opaqueCulling;
    Gfx::PDescriptorSet transparentCulling;

    // use a different texture here so we can do multisampling
    Gfx::OTexture2D msBasePassDepth;
    Gfx::OTexture2D basePassDepth;
    // hdr
    Gfx::OTexture2D msBasePassColor;
    Gfx::OTexture2D basePassColor;

    // used for transparency sorting
    Vector cameraPos;
    Vector cameraForward;
    Gfx::PDescriptorSet viewParamsSet;

    PCameraActor source;
    Gfx::OPipelineLayout basePassLayout;
    Gfx::ODescriptorLayout lightCullingLayout;

    Gfx::OPipelineStatisticsQuery query;
    Gfx::PTimestampQuery timestamps;

    Gfx::PShaderBuffer cullingBuffer;

    //OWaterRenderer waterRenderer;
    //OTerrainRenderer terrainRenderer;

    // Debug rendering
    Gfx::OVertexInput debugVertexInput;
    Gfx::OVertexBuffer debugVertices;
    Gfx::OVertexShader debugVertexShader;
    Gfx::OFragmentShader debugFragmentShader;
    Gfx::PGraphicsPipeline debugPipeline;
    Gfx::OPipelineLayout debugPipelineLayout;

    // Skybox
    Gfx::ODescriptorLayout skyboxDataLayout;
    Gfx::PDescriptorSet skyboxDataSet;
    Gfx::ODescriptorLayout textureLayout;
    Gfx::PDescriptorSet textureSet;
    Gfx::OVertexShader vertexShader;
    Gfx::OFragmentShader fragmentShader;
    Gfx::OPipelineLayout pipelineLayout;
    Gfx::PGraphicsPipeline skyboxPipeline;
    Gfx::OSampler skyboxSampler;
    struct SkyboxData {
        Matrix4 transformMatrix;
        Vector fogColor;
        float blendFactor;
    } skyboxData;
    Component::Skybox skybox;
    const char* SKYBOXDAY_NAME = "day";
    const char* SKYBOXNIGHT_NAME = "night";
    const char* SKYBOXSAMPLER_NAME = "sampler";
    PScene scene;
};
DEFINE_REF(BasePass)
} // namespace Seele
