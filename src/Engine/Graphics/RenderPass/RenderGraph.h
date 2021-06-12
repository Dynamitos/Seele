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
    void addRenderPass(PRenderPass renderPass);
    Gfx::PRenderTargetAttachment requestRenderTarget(const std::string& outputName);
    Gfx::PTexture requestTexture(const std::string& outputName);
    Gfx::PStructuredBuffer requestBuffer(const std::string& outputName);
    Gfx::PUniformBuffer requestUniform(const std::string& outputName);
    void registerRenderPassOutput(const std::string& outputName, Gfx::PRenderTargetAttachment attachment);
    void registerTextureOutput(const std::string& outputName, Gfx::PTexture buffer);
    void registerBufferOutput(const std::string& outputName, Gfx::PStructuredBuffer buffer);
    void registerUniformOutput(const std::string& outputName, Gfx::PUniformBuffer buffer);
private:
    Map<std::string, Gfx::PRenderTargetAttachment> registeredAttachments;
    Map<std::string, Gfx::PTexture> registeredTextures;
    Map<std::string, Gfx::PStructuredBuffer> registeredBuffers;
    Map<std::string, Gfx::PUniformBuffer> registeredUniforms;
    List<PRenderPass> renderPasses;
};
DEFINE_REF(RenderGraph)
} // namespace Seele
