#pragma once
#include "RenderPass.h"
#include "Graphics/Resources.h"
#include "Component/Skybox.h"

namespace Seele
{
DECLARE_REF(CameraActor)
DECLARE_REF(Scene)
DECLARE_REF(Viewport)
class SkyboxRenderPass : public RenderPass
{
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
    Gfx::OVertexBuffer cubeBuffer;
    Gfx::OUniformBuffer viewParamsBuffer;
    Gfx::ODescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;
    Gfx::OPipelineLayout pipelineLayout;
    Gfx::OVertexDeclaration declaration;
    Gfx::OVertexShader vertexShader;
    Gfx::OFragmentShader fragmentShader;
    Gfx::OGraphicsPipeline pipeline;
    Gfx::OSamplerState skyboxSampler;
    Component::Skybox skybox;
};
DEFINE_REF(SkyboxRenderPass)
} // namespace Seele
