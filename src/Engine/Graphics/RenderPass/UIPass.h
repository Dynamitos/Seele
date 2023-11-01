#pragma once
#include "RenderPass.h"
#include "UI/RenderHierarchy.h"
#include "Graphics/Resources.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture2D)
DECLARE_NAME_REF(Gfx, RenderTargetAttachment)
class UIPass : public RenderPass
{
public:
    UIPass(Gfx::PGraphics graphics, PScene scene);
    virtual ~UIPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::ORenderTargetAttachment renderTarget;
    Gfx::OTexture2D colorBuffer;
    Gfx::ORenderTargetAttachment depthAttachment;
    Gfx::OTexture2D depthBuffer;

    Gfx::ODescriptorLayout descriptorLayout;
    Gfx::ODescriptorSet descriptorSet;

    Gfx::OUniformBuffer numTexturesBuffer;
    Gfx::OVertexBuffer elementBuffer;

    Gfx::OVertexDeclaration declaration;
    Gfx::OVertexShader vertexShader;
    Gfx::OFragmentShader fragmentShader;
    Gfx::OPipelineLayout pipelineLayout;
    Gfx::OGraphicsPipeline pipeline;

    Array<UI::RenderElementStyle> renderElements;
    Array<Gfx::PTexture> usedTextures;
};
DEFINE_REF(UIPass);
} // namespace Seele
