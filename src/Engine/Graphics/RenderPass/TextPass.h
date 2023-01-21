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
    Vector4 textColor;
    Vector2 position;
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
        Vector2 bearing;
        Vector2 size;
        uint32 advance;
    };
    struct GlyphInstanceData
    {
        Vector2 position;
        Vector2 widthHeight;
        uint32 glyphIndex;
    };
    struct TextData
    {
        Vector4 textColor;
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
