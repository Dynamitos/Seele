#pragma once
#include "MinimalEngine.h"
#include "RenderPass.h"
#include "WaterRenderer.h"
#include "TerrainRenderer.h"

namespace Seele {
DECLARE_REF(CameraActor)
class BasePass : public RenderPass {
  public:
    BasePass(Gfx::PGraphics graphics, PScene scene);
    BasePass(BasePass&&) = default;
    BasePass& operator=(BasePass&&) = default;
    virtual ~BasePass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    Gfx::RenderTargetAttachment msColorAttachment;
    Gfx::RenderTargetAttachment colorAttachment;
    Gfx::RenderTargetAttachment msDepthAttachment;
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::PShaderBuffer oLightIndexList;
    Gfx::PShaderBuffer tLightIndexList;
    Gfx::PTexture2D oLightGrid;
    Gfx::PTexture2D tLightGrid;

    Gfx::PDescriptorSet opaqueCulling;
    Gfx::PDescriptorSet transparentCulling;

    // use a different texture here so we can do multisampling
    Gfx::OTexture2D msBasePassDepth;
    Gfx::OTexture2D basePassDepth;
    Gfx::OTexture2D msBasePassColor;

    // used for transparency sorting
    Vector cameraPos;
    Vector cameraForward;

    PCameraActor source;
    Gfx::OPipelineLayout basePassLayout;
    Gfx::ODescriptorLayout lightCullingLayout;

    Gfx::OPipelineStatisticsQuery query;
    Gfx::PTimestampQuery timestamps;

    Gfx::PShaderBuffer cullingBuffer;

    //OWaterRenderer waterRenderer;
    OTerrainRenderer terrainRenderer;

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
    Gfx::PGraphicsPipeline pipeline;
    Gfx::OSampler skyboxSampler;
    struct SkyboxData {
        Matrix4 transformMatrix;
        Vector fogColor;
        float blendFactor;
    } skyboxData;
    Gfx::OUniformBuffer skyboxBuffer;
    Component::Skybox skybox;
};
DEFINE_REF(BasePass)
} // namespace Seele
