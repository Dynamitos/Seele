#pragma once
#include "Component/Skybox.h"
#include "Graphics/Shader.h"
#include "RenderPass.h"


namespace Seele {
class SkyboxRenderPass : public RenderPass {
  public:
    SkyboxRenderPass(Gfx::PGraphics graphics, PScene scene);
    SkyboxRenderPass(SkyboxRenderPass&&) = default;
    SkyboxRenderPass& operator=(SkyboxRenderPass&&) = default;
    virtual ~SkyboxRenderPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    Gfx::RenderTargetAttachment colorAttachment;
    Gfx::RenderTargetAttachment depthAttachment;
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
DEFINE_REF(SkyboxRenderPass)
} // namespace Seele
