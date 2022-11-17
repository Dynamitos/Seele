#pragma once
#include "RenderPass.h"
#include "UI/RenderHierarchy.h"
#include "Graphics/GraphicsResources.h"
#include "Asset/FontAsset.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture2D)
DECLARE_NAME_REF(Gfx, RenderTargetAttachment)
DECLARE_NAME_REF(Gfx, StructuredBuffer)
struct TextRender
{
    std::string text;
    PFontAsset font;
    Math::Vector4 textColor;
    Math::Vector2 position;
    float scale;
};
struct TextPassData
{
    Array<TextRender> texts;
};

class TextPass : public RenderPass<TextPassData>
{
public:
    TextPass(Gfx::PGraphics graphics);
    virtual ~TextPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    struct GlyphData
    {
        Math::Vector2 bearing;
        Math::Vector2 size;
        uint32 advance;
    };
    struct GlyphInstanceData
    {
        Math::Vector2 position;
        Math::Vector2 widthHeight;
        uint32 glyphIndex;
    };
    struct TextData
    {
        Math::Vector4 textColor;
        float scale;
    };
    struct FontData
    {
        Gfx::PDescriptorSet textureArraySet;
        Array<GlyphData> glyphDataSet;
        Map<uint32, uint32> characterToGlyphIndex;
    };
    FontData& getFontData(PFontAsset font);
    Map<PFontAsset, FontData> fontData;

    struct TextResources
    {
        Gfx::PVertexBuffer vertexBuffer;
        Gfx::PDescriptorSet textureArraySet;
        TextData textData;
    };
    std::map<PFontAsset, Array<TextResources>> textResources;

    Gfx::PRenderTargetAttachment renderTarget;
    Gfx::PRenderTargetAttachment depthAttachment;

    Gfx::PDescriptorLayout generalLayout;
    Gfx::PDescriptorLayout textureArrayLayout;

    Gfx::PDescriptorSet generalSet;

    Gfx::PUniformBuffer projectionBuffer;
    Gfx::PSamplerState glyphSampler;

    Gfx::PVertexDeclaration declaration;
    Gfx::PVertexShader vertexShader;
    Gfx::PFragmentShader fragmentShader;
    Gfx::PPipelineLayout pipelineLayout;
    Gfx::PGraphicsPipeline pipeline;
};
DEFINE_REF(TextPass);
} // namespace Seele
