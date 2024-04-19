#pragma once
#include "RenderPass.h"
#include "UI/RenderHierarchy.h"
#include "Asset/FontAsset.h"
#include "Graphics/Shader.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture2D)
DECLARE_NAME_REF(Gfx, ShaderBuffer)
struct TextRender
{
    std::string text;
    PFontAsset font;
    Vector4 textColor;
    Vector2 position;
    float scale;
};
class TextPass : public RenderPass
{
public:
    TextPass(Gfx::PGraphics graphics, PScene scene);
    TextPass(TextPass&&) = default;
    TextPass& operator=(TextPass&&) = default;
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
        Gfx::PShaderBuffer instanceBuffer;
        Gfx::PDescriptorSet textureArraySet;
        TextData textData;
    };
    Map<PFontAsset, Array<TextResources>> textResources;

    Gfx::RenderTargetAttachment renderTarget;
    Gfx::RenderTargetAttachment depthAttachment;

    Gfx::ODescriptorLayout generalLayout;
    Gfx::ODescriptorLayout textureArrayLayout;

    Gfx::PDescriptorSet generalSet;

    Gfx::OUniformBuffer projectionBuffer;
    Gfx::OSampler glyphSampler;

    Gfx::OVertexShader vertexShader;
    Gfx::OFragmentShader fragmentShader;
    Gfx::OPipelineLayout pipelineLayout;
    Gfx::PGraphicsPipeline pipeline;
    Array<TextRender> texts;
};
DEFINE_REF(TextPass);
} // namespace Seele
