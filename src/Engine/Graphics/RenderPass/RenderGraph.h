#pragma once
#include "MinimalEngine.h"
#include "Graphics/GraphicsResources.h"
#include "RenderPass.h"

namespace Seele
{
class RenderGraph
{
public:
    RenderGraph();
    ~RenderGraph();
    void setup();
    Gfx::PRenderTargetAttachment requestRenderTarget(const std::string& outputName);
    void registerRenderPassOutput(const std::string& ouputName, Gfx::PRenderTargetAttachment attachment);
private:
    Map<std::string, Gfx::PRenderTargetAttachment> registeredAttachments;
    List<PRenderPass> renderPasses;
};
DEFINE_REF(RenderGraph)
} // namespace Seele
