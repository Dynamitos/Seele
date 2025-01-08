#pragma once
#include "Graphics/Shader.h"
#include "RenderPass.h"
#include "Asset/FontAsset.h"
#include "UI/System.h"

namespace Seele {
DECLARE_NAME_REF(Gfx, Texture2D)
DECLARE_NAME_REF(Gfx, RenderTargetAttachment)
struct TextRender {
    std::string text;
    PFontAsset font;
    uint32 fontSize;
    Vector4 textColor;
    Vector2 position;
};
class UIPass : public RenderPass {
  public:
    UIPass(Gfx::PGraphics graphics, UI::PSystem system);
    UIPass(UIPass&&) = default;
    UIPass& operator=(UIPass&&) = default;
    virtual ~UIPass();
    virtual void beginFrame(const Component::Camera& cam) override;
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
        float width;
        float height;
        uint32 glyphIndex;
    };
    struct TextData {
        Vector4 textColor;
        float scale;
    };

    struct TextResources {
        Gfx::PShaderBuffer instanceBuffer;
        Gfx::PDescriptorSet textureArraySet;
        TextData textData;
    };
    Map<PFontAsset, Array<TextResources>> textResources;

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

    Array<TextRender> texts;
    Array<GlyphInstanceData> glyphs;
    Gfx::OShaderBuffer glyphInstanceBuffer;
    Gfx::OSampler glyphSampler;
    Array<Gfx::PTexture2D> usedTextures;
};
DEFINE_REF(UIPass);
} // namespace Seele
