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
    Gfx::PRenderTargetAttachment renderTarget;
    Gfx::PTexture2D colorBuffer;
    Gfx::PRenderTargetAttachment depthAttachment;
    Gfx::PTexture2D depthBuffer;

    Gfx::PDescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;

    Gfx::PUniformBuffer numTexturesBuffer;
    Gfx::PVertexBuffer elementBuffer;

    Gfx::PVertexDeclaration declaration;
    Gfx::PVertexShader vertexShader;
    Gfx::PFragmentShader fragmentShader;
    Gfx::PPipelineLayout pipelineLayout;
    Gfx::PGraphicsPipeline pipeline;

    Array<UI::RenderElementStyle> renderElements;
    Array<Gfx::PTexture> usedTextures;
};
DEFINE_REF(UIPass);
} // namespace Seele
