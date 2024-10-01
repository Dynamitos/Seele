#include "TextPass.h"
#include "Graphics/Command.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderTarget.h"
#include "RenderGraph.h"

using namespace Seele;

TextPass::TextPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics, scene) {}

TextPass::~TextPass() {}

void TextPass::beginFrame(const Component::Camera& cam) {
    RenderPass::beginFrame(cam);
    for (TextRender& render : texts) {
        FontData& fd = getFontData(render.font);
        TextResources& res = textResources[render.font].add();
        Array<GlyphInstanceData> instanceData;
        float x = render.position.x;
        float y = render.position.y;
        for (uint32 c : render.text) {
            const GlyphData& glyph = fd.glyphDataSet[fd.characterToGlyphIndex[c]];
            Vector2 bearing = glyph.bearing;
            Vector2 size = glyph.size;
            float xpos = x + bearing.x * render.scale;
            float ypos = y - (size.y - bearing.y) * render.scale;

            float w = size.x * render.scale;
            float h = size.y * render.scale;

            instanceData.add(GlyphInstanceData{
                .position = Vector2(xpos, ypos),
                .widthHeight = Vector2(w, h),
                .glyphIndex = fd.characterToGlyphIndex[c],
            });
            x += (glyph.advance >> 6) * render.scale;
        }
        res.instanceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
            .sourceData =
                {
                    .size = static_cast<uint32>(instanceData.size() * sizeof(GlyphInstanceData)),
                    .data = reinterpret_cast<uint8*>(instanceData.data()),
                },
            .numElements = instanceData.size(),
        });

        res.textureArraySet = fd.textureArraySet;

        res.textData = {
            .textColor = render.textColor,
            .scale = render.scale,
        };
    }
    auto proj = viewport->getProjectionMatrix();
    projectionBuffer->updateContents(0, sizeof(Matrix4), &proj);
    generalSet->updateBuffer(1, 0, projectionBuffer);
    generalSet->writeChanges();
    // co_return;
}

void TextPass::render() {
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;
    for (const auto& [fontAsset, res] : textResources) {
        Gfx::ORenderCommand command = graphics->createRenderCommand("TextPassCommand");
        command->setViewport(viewport);
        command->bindPipeline(pipeline);
        for (const auto& resource : res) {
            command->bindDescriptor({generalSet, resource.textureArraySet});
            // command->bindVertexBuffer({resource.vertexBuffer});

            command->pushConstants(Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(TextData),
                                   &resource.textData);
            // command->draw(4, static_cast<uint32>(resource.vertexBuffer->getNumVertices()), 0, 0);
        }
        commands.add(std::move(command));
    }
    graphics->executeCommands(std::move(commands));
    graphics->endRenderPass();
    textResources.clear();
    // co_return;
}

void TextPass::endFrame() {
    // co_return;
}

void TextPass::publishOutputs() {}

void TextPass::createRenderPass() {
    renderTarget = resources->requestRenderTarget("UIPASS_COLOR");
    depthAttachment = resources->requestRenderTarget("UIPASS_DEPTH");

    ShaderCompilationInfo createInfo = {
        .name = "TextVertex",
        .modules = {"TextPass"},
        .entryPoints = {{"vertexMain", "TextPass"}, {"fragmentMain", "TextFragment"}},
    };
    graphics->beginShaderCompilation(createInfo);
    vertexShader = graphics->createVertexShader({0});
    fragmentShader = graphics->createFragmentShader({1});

    generalLayout = graphics->createDescriptorLayout("pRender");
    generalLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    });
    generalLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    generalLayout->create();

    textureArrayLayout = graphics->createDescriptorLayout("pGlyphTextures");
    textureArrayLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .textureType = Gfx::SE_IMAGE_VIEW_TYPE_2D_ARRAY,
        .descriptorCount = 256,
        .bindingFlags = Gfx::SE_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
    });
    textureArrayLayout->create();

    projectionBuffer = graphics->createUniformBuffer({
        .sourceData =
            {
                .size = sizeof(Matrix4),
                .data = nullptr,
            },
    });

    glyphSampler = graphics->createSampler({
        .magFilter = Gfx::SE_FILTER_LINEAR,
        .minFilter = Gfx::SE_FILTER_LINEAR,
        .addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    });

    generalSet = generalLayout->allocateDescriptorSet();
    generalSet->updateBuffer(0, 0, projectionBuffer);
    generalSet->updateSampler(1, 0, glyphSampler);
    generalSet->writeChanges();

    pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(generalLayout);
    pipelineLayout->addDescriptorLayout(textureArrayLayout);
    pipelineLayout->addPushConstants(
        {.stageFlags = Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT, .offset = 0, .size = sizeof(TextData)});
    pipelineLayout->create();

    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{.colorAttachments = {renderTarget}, .depthAttachment = depthAttachment};
    renderPass = graphics->createRenderPass(std::move(layout), {}, viewport);

    Gfx::LegacyPipelineCreateInfo pipelineInfo;
    pipelineInfo.vertexShader = vertexShader;
    pipelineInfo.fragmentShader = fragmentShader;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pipelineLayout = pipelineLayout;
    pipelineInfo.rasterizationState.cullMode = Gfx::SE_CULL_MODE_NONE;
    pipelineInfo.colorBlend.attachmentCount = 1;
    pipelineInfo.colorBlend.blendAttachments[0].blendEnable = true;
    pipelineInfo.colorBlend.blendAttachments[0].colorWriteMask =
        Gfx::SE_COLOR_COMPONENT_R_BIT | Gfx::SE_COLOR_COMPONENT_G_BIT | Gfx::SE_COLOR_COMPONENT_B_BIT | Gfx::SE_COLOR_COMPONENT_A_BIT;
    pipelineInfo.colorBlend.blendAttachments[0].srcColorBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
    pipelineInfo.colorBlend.blendAttachments[0].dstColorBlendFactor = Gfx::SE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineInfo.colorBlend.blendAttachments[0].alphaBlendOp = Gfx::SE_BLEND_OP_ADD;
    pipelineInfo.colorBlend.blendAttachments[0].srcAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineInfo.colorBlend.blendAttachments[0].dstAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_ZERO;
    pipelineInfo.colorBlend.blendAttachments[0].alphaBlendOp = Gfx::SE_BLEND_OP_ADD;
    pipelineInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
}
TextPass::FontData& TextPass::getFontData(PFontAsset font) {
    if (fontData.exists(font)) {
        return fontData[font];
    }
    const auto& fontGlyphs = font->getGlyphData();
    FontData& fd = fontData[font];
    Array<GlyphData> glyphData;
    Array<Gfx::PTexture2D> textures;
    glyphData.reserve(fontGlyphs.size());
    for (const auto& [key, value] : fontGlyphs) {
        fd.characterToGlyphIndex[key] = static_cast<uint32>(glyphData.size());
        GlyphData& gd = glyphData.add();
        gd.bearing = value.bearing;
        gd.size = value.size;
        gd.advance = value.advance;
        textures.add(font->getTexture(value.textureIndex));
    }
    fd.glyphDataSet = glyphData;

    textureArrayLayout->reset();
    fd.textureArraySet = textureArrayLayout->allocateDescriptorSet();
    for (uint32 i = 0; i < textures.size(); ++i) {
        fd.textureArraySet->updateTexture(0, i, textures[i]);
    }
    fd.textureArraySet->writeChanges();
    return fontData[font];
}
