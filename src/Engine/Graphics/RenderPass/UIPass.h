#pragma once
#include "Graphics/Shader.h"
#include "RenderPass.h"
#include "Asset/FontAsset.h"
#include "UI/System.h"

namespace Seele {
DECLARE_NAME_REF(Gfx, Texture2D)
DECLARE_NAME_REF(Gfx, RenderTargetAttachment)
class UIPass : public RenderPass {
  public:
    UIPass(Gfx::PGraphics graphics, UI::PSystem system);
    UIPass(UIPass&&) = default;
    UIPass& operator=(UIPass&&) = default;
    virtual ~UIPass();
    virtual void beginFrame(const Component::Camera& cam, const Component::Transform& transform) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    struct GlyphData {
        Vector2 bearing;
        Vector2 size;
        uint32 advance;
    };
    struct GlyphInstanceData {
        float x;
        float y;
        float z;
        float width;
        float height;
        uint32 glyphIndex;
    };

    struct RenderElementStyle {
        float x;
        float y;
        float w;
        float h;
        Vector color;
        float opacity = 1;
        float z = 0;
        uint32 textureIndex = std::numeric_limits<uint32>::max();
        uint32 pad0;
        uint32 pad1;
    };

    Gfx::RenderTargetAttachment colorAttachment;
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::OTexture2D depthBuffer;

    Gfx::ODescriptorLayout textDescriptorLayout;
    Gfx::PDescriptorSet textDescriptorSet;

    Gfx::OVertexShader textVertexShader;
    Gfx::OFragmentShader textFragmentShader;
    Gfx::OPipelineLayout textPipelineLayout;
    Gfx::PGraphicsPipeline textPipeline;

    Gfx::ODescriptorLayout uiDescriptorLayout;
    Gfx::PDescriptorSet uiDescriptorSet;

    Gfx::OVertexShader uiVertexShader;
    Gfx::OFragmentShader uiFragmentShader;
    Gfx::OPipelineLayout uiPipelineLayout;
    Gfx::PGraphicsPipeline uiPipeline;

    UI::PSystem system;
    Array<UI::UIRender> renderElements;
    Array<GlyphInstanceData> glyphs;
    Array<RenderElementStyle> elements;
    Gfx::OShaderBuffer glyphInstanceBuffer;
    constexpr static const char* GLYPHINSTANCE_NAME = "glyphData";
    Gfx::OShaderBuffer elementBuffer;
    constexpr static const char* ELEMENT_NAME = "elements";
    Gfx::OSampler glyphSampler;
    constexpr static const char* GLYPHSAMPLER_NAME = "glyphSampler";
    Array<Gfx::PTexture2D> usedTextures;
    constexpr static const char* TEXTURES_NAME = "textures";
};
DEFINE_REF(UIPass);
} // namespace Seele
