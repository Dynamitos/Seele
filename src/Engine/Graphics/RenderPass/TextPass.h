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
    TextPass(Gfx::PGraphics graphics, Gfx::PViewport viewport, Gfx::PRenderTargetAttachment renderTarget);
    virtual ~TextPass();
    virtual MainJob beginFrame() override;
    virtual MainJob render() override;
    virtual MainJob endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    struct GlyphData
    {
        Vector2 bearing;
        Vector2 size;
    };
    struct GlyphInstanceData
    {
        uint32 glyphIndex;
        Vector2 position;
    };
    struct FontData
    {
        Gfx::PStructuredBuffer structuredBuffer;
        Array<Gfx::PTexture> textures;
        Map<uint32, uint32> characterToGlyphIndex;
        // Logically this should be part of GlyphData,
        // but because GlyphData mirrors shader data and we need
        // the advance on the CPU, we need this in a separate structure
        Map<uint32, uint32> characterAdvance;
    };
    FontData& getFontData(PFontAsset font);
    Map<PFontAsset, FontData> fontData;

    struct TextResources
    {
        Gfx::PDescriptorSet descriptorSet;
        Gfx::PDescriptorSet textureArraySet;
        Gfx::PVertexBuffer vertexBuffer;
        float scale;
    };
    Array<TextResources> textResources;

    Gfx::PRenderTargetAttachment renderTarget;
    Gfx::PRenderTargetAttachment depthAttachment;
    Gfx::PTexture2D depthBuffer;

    Gfx::PDescriptorLayout descriptorLayout;
    Gfx::PDescriptorLayout textureArrayLayout;

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
