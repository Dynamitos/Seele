#pragma once
#include "MinimalEngine.h"
#include "Graphics/GraphicsResources.h"

namespace Seele
{
DECLARE_REF(ViewFrame)

class RenderGraphResources
{
public:
    RenderGraphResources();
    ~RenderGraphResources();
    Gfx::PRenderTargetAttachment requestRenderTarget(const std::string& outputName);
    Gfx::PTexture requestTexture(const std::string& outputName);
    Gfx::PShaderBuffer requestBuffer(const std::string& outputName);
    Gfx::PUniformBuffer requestUniform(const std::string& outputName);
    void registerRenderPassOutput(const std::string& outputName, Gfx::PRenderTargetAttachment attachment);
    void registerTextureOutput(const std::string& outputName, Gfx::PTexture buffer);
    void registerBufferOutput(const std::string& outputName, Gfx::PShaderBuffer buffer);
    void registerUniformOutput(const std::string& outputName, Gfx::PUniformBuffer buffer);
protected:
    Map<std::string, Gfx::PRenderTargetAttachment> registeredAttachments;
    Map<std::string, Gfx::PTexture> registeredTextures;
    Map<std::string, Gfx::PShaderBuffer> registeredBuffers;
    Map<std::string, Gfx::PUniformBuffer> registeredUniforms;
};
DEFINE_REF(RenderGraphResources)
} // namespace Seele
