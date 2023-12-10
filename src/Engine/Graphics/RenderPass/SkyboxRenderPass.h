#pragma once
#include "RenderPass.h"
#include "Graphics/Shader.h"
#include "Component/Skybox.h"

namespace Seele
{
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
    Gfx::PRenderTargetAttachment baseColorAttachment;
    Gfx::OVertexBuffer cubeBuffer;
    Gfx::ODescriptorLayout skyboxDataLayout;
    Gfx::PDescriptorSet skyboxDataSet;
    Gfx::ODescriptorLayout textureLayout;
    Gfx::PDescriptorSet textureSet;
    Gfx::OVertexShader vertexShader;
    Gfx::OFragmentShader fragmentShader;
    Gfx::PGraphicsPipeline pipeline;
    Gfx::OSampler skyboxSampler;
    Component::Skybox skybox;
};
DEFINE_REF(SkyboxRenderPass)
} // namespace Seele
