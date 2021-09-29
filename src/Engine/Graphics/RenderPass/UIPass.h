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
class UIPass : public RenderPass
{
public:
    UIPass(PRenderGraph renderGraph, Gfx::PGraphics graphics, Gfx::PViewport viewport, Gfx::PRenderTargetAttachment renderTarget);
    virtual ~UIPass();
    virtual void beginFrame() override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    PUIViewFrame uiFrame;
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
