#pragma once
#include "Graphics/Buffer.h"
#include "Graphics/Query.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/Texture.h"
#include "MinimalEngine.h"


namespace Seele {
DECLARE_REF(ViewFrame)

class RenderGraphResources {
  public:
    RenderGraphResources();
    ~RenderGraphResources();
    Gfx::RenderTargetAttachment requestRenderTarget(const std::string& outputName);
    Gfx::PTexture requestTexture(const std::string& outputName);
    Gfx::PShaderBuffer requestBuffer(const std::string& outputName);
    Gfx::PUniformBuffer requestUniform(const std::string& outputName);
    Gfx::PPipelineStatisticsQuery requestQuery(const std::string& outputName);
    void registerRenderPassOutput(const std::string& outputName, Gfx::RenderTargetAttachment attachment);
    void registerTextureOutput(const std::string& outputName, Gfx::PTexture buffer);
    void registerBufferOutput(const std::string& outputName, Gfx::PShaderBuffer buffer);
    void registerUniformOutput(const std::string& outputName, Gfx::PUniformBuffer buffer);
    void registerQueryOutput(const std::string& outputName, Gfx::PPipelineStatisticsQuery query);

  protected:
    Map<std::string, Gfx::RenderTargetAttachment> registeredAttachments;
    Map<std::string, Gfx::PTexture> registeredTextures;
    Map<std::string, Gfx::PShaderBuffer> registeredBuffers;
    Map<std::string, Gfx::PUniformBuffer> registeredUniforms;
    Map<std::string, Gfx::PPipelineStatisticsQuery> registeredQueries;
};
DEFINE_REF(RenderGraphResources)
} // namespace Seele
