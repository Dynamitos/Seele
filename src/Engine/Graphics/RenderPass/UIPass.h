#pragma once
#include "RenderPass.h"
#include "UI/RenderHierarchy.h"
#include "Graphics/GraphicsResources.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture2D)
DECLARE_NAME_REF(Gfx, RenderTargetAttachment)
struct UIPassData
{
    const UI::RenderHierarchy hierarchy;
};
class UIPass : public RenderPass<UIPassData>
{
public:
    UIPass(Gfx::PGraphics graphics, Gfx::PViewport viewport, Gfx::PRenderTargetAttachment renderTarget);
    virtual ~UIPass();
    virtual MainJob beginFrame() override;
    virtual MainJob render() override;
    virtual MainJob endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::PRenderTargetAttachment renderTarget;
    Gfx::PRenderTargetAttachment depthAttachment;
    Gfx::PTexture2D depthBuffer;

    Gfx::PVertexDeclaration declaration;
    Gfx::PVertexShader vertexShader;
    Gfx::PFragmentShader fragmentShader;
    Gfx::PPipelineLayout pipelineLayout;
    Gfx::PGraphicsPipeline pipeline;
};
DEFINE_REF(UIPass);
} // namespace Seele
