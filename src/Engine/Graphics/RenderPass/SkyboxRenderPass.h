#pragma once
#include "RenderPass.h"
#include "Graphics/GraphicsResources.h"
#include "Component/Skybox.h"

namespace Seele
{
DECLARE_REF(CameraActor)
DECLARE_REF(Scene)
DECLARE_REF(Viewport)
struct SkyboxPassData
{
    Component::Skybox skybox;
};
class SkyboxRenderPass : public RenderPass<SkyboxPassData>
{
public:
    SkyboxRenderPass(Gfx::PGraphics graphics);
    virtual ~SkyboxRenderPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::PVertexBuffer cubeBuffer;
    Gfx::PUniformBuffer viewParamsBuffer;
    Gfx::PDescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;
    Gfx::PPipelineLayout pipelineLayout;
    Gfx::PGraphicsPipeline pipeline;
};
DEFINE_REF(SkyboxRenderPass)
} // namespace Seele
