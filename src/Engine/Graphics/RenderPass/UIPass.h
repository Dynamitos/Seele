#pragma once
#include "RenderPass.h"
#include "UI/RenderHierarchy.h"
#include "Graphics/Shader.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture2D)
DECLARE_NAME_REF(Gfx, RenderTargetAttachment)
class UIPass : public RenderPass
{
public:
    UIPass(Gfx::PGraphics graphics, PScene scene);
    UIPass(UIPass&&) = default;
    UIPass& operator=(UIPass&&) = default;
    virtual ~UIPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::RenderTargetAttachment renderTarget;
    Gfx::OTexture2D colorBuffer;
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::OTexture2D depthBuffer;

    Gfx::ODescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;

    Gfx::OUniformBuffer numTexturesBuffer;
    Gfx::OVertexBuffer elementBuffer;

    Gfx::OVertexShader vertexShader;
    Gfx::OFragmentShader fragmentShader;
    Gfx::PGraphicsPipeline pipeline;
    Gfx::OPipelineLayout pipelineLayout;

    Array<UI::RenderElementStyle> renderElements;
    Array<Gfx::PTexture> usedTextures;
};
DEFINE_REF(UIPass);
} // namespace Seele
