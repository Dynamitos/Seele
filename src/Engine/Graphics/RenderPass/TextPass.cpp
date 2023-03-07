#include "TextPass.h"
#include "RenderGraph.h"
#include "Graphics/Graphics.h"
#include "Graphics/VertexShaderInput.h"

using namespace Seele;

TextPass::TextPass(Gfx::PGraphics graphics) 
    : RenderPass(graphics)
{
}

TextPass::~TextPass()
{
    
}

void TextPass::beginFrame(const Component::Camera&) 
{
    for(TextRender& render : passData.texts)
    {
        FontData& fd = getFontData(render.font);
        TextResources& res = textResources[render.font].add();
        Array<GlyphInstanceData> instanceData;
        float x = render.position.x;
        float y = render.position.y;
        for(uint32 c : render.text)
        {
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
        VertexBufferCreateInfo vbInfo = {
            .resourceData = {
                .size = static_cast<uint32>(instanceData.size() * sizeof(GlyphInstanceData)),
                .data = reinterpret_cast<uint8*>(instanceData.data()),
                },
            .vertexSize = sizeof(GlyphInstanceData),
            .numVertices = static_cast<uint32>(instanceData.size()),
        };
        res.vertexBuffer = graphics->createVertexBuffer(vbInfo);

        res.textureArraySet = fd.textureArraySet;

        res.textData = {
            .textColor = render.textColor,
            .scale = render.scale,
        };
    }
    auto proj = viewport->getProjectionMatrix();
    BulkResourceData projectionUpdate = {
        .size = sizeof(Matrix4),
        .data = (uint8*)&proj,
    };
    projectionBuffer->updateContents(projectionUpdate);
    generalSet->updateBuffer(1, projectionBuffer);
    generalSet->writeChanges();
    //co_return;
}

void TextPass::render() 
{
    graphics->beginRenderPass(renderPass);
    Array<Gfx::PRenderCommand> commands;
    for(const auto& [fontAsset, res] : textResources)
    {
        Gfx::PRenderCommand command = graphics->createRenderCommand("TextPassCommand");
        command->setViewport(viewport);
        command->bindPipeline(pipeline);
        for(const auto& resource : res)
        {
            command->bindDescriptor({generalSet, resource.textureArraySet});
            command->bindVertexBuffer({VertexInputStream(0, 0, resource.vertexBuffer)});
            
            command->pushConstants(pipelineLayout, Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(TextData), &resource.textData);
            command->draw(4, static_cast<uint32>(resource.vertexBuffer->getNumVertices()), 0, 0);
        }
        commands.add(command);
    }
    graphics->executeCommands(commands);
    graphics->endRenderPass();
    textResources.clear();
    //co_return;
}

void TextPass::endFrame() 
{
    //co_return;
}

void TextPass::publishOutputs() 
{
}

void TextPass::createRenderPass() 
{
    renderTarget = resources->requestRenderTarget("UIPASS_COLOR");
    depthAttachment = resources->requestRenderTarget("UIPASS_DEPTH");
    
    ShaderCreateInfo createInfo;
    createInfo.mainModule = "TextPass";
    createInfo.defines["INDEX_VIEW_PARAMS"] = "0";
    createInfo.name = "TextVertex";
    createInfo.entryPoint = "vertexMain";
    vertexShader = graphics->createVertexShader(createInfo);

    createInfo.name = "TextFragment";
    createInfo.entryPoint =  "fragmentMain";
    fragmentShader = graphics->createFragmentShader(createInfo);
    Array<Gfx::VertexElement> elements;
    elements.add({
        .streamIndex = 0, 
        .offset = offsetof(GlyphInstanceData, position), 
        .vertexFormat = Gfx::SE_FORMAT_R32G32_SFLOAT, 
        .attributeIndex = 0, 
        .stride = sizeof(GlyphInstanceData), 
        .bInstanced = 1
    });
    elements.add({
        .streamIndex = 0, 
        .offset = offsetof(GlyphInstanceData, widthHeight), 
        .vertexFormat = Gfx::SE_FORMAT_R32G32_SFLOAT, 
        .attributeIndex = 1,
        .stride = sizeof(GlyphInstanceData), 
        .bInstanced = 1
    });
    elements.add({
        .streamIndex = 0,
        .offset = offsetof(GlyphInstanceData, glyphIndex),
        .vertexFormat = Gfx::SE_FORMAT_R32_UINT,
        .attributeIndex = 2,
        .stride = sizeof(GlyphInstanceData),
        .bInstanced = 1
    });
    declaration = graphics->createVertexDeclaration(elements);

    generalLayout = graphics->createDescriptorLayout("TextGeneral");
    generalLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    generalLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
    generalLayout->create();

    textureArrayLayout = graphics->createDescriptorLayout("TextTextureArray");
    textureArrayLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 256, Gfx::SE_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);
    textureArrayLayout->create();

    projectionBuffer = graphics->createUniformBuffer({
        .resourceData = {
            .size = sizeof(Matrix4),
            .data = nullptr,
        },
        .bDynamic = true,
    });

    glyphSampler = graphics->createSamplerState({
        .magFilter = Gfx::SE_FILTER_LINEAR,
        .minFilter = Gfx::SE_FILTER_LINEAR,
        .addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    });

    generalSet = generalLayout->allocateDescriptorSet();
    generalSet->updateBuffer(0, projectionBuffer);
    generalSet->updateSampler(1, glyphSampler);
    generalSet->writeChanges();
    
    pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, generalLayout);
    pipelineLayout->addDescriptorLayout(1, textureArrayLayout);
    pipelineLayout->addPushConstants({
        .stageFlags = Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT, 
        .offset = 0, 
        .size = sizeof(TextData)});
    pipelineLayout->create();

    Gfx::PRenderTargetLayout layout = new Gfx::RenderTargetLayout(renderTarget, depthAttachment);
    renderPass = graphics->createRenderPass(layout, viewport);
    
    GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.vertexDeclaration = declaration;
    pipelineInfo.vertexShader = vertexShader;
    pipelineInfo.fragmentShader = fragmentShader;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.pipelineLayout = pipelineLayout;
    pipelineInfo.rasterizationState.cullMode = Gfx::SE_CULL_MODE_NONE;
    pipelineInfo.colorBlend.attachmentCount = 1;
    pipelineInfo.colorBlend.blendAttachments[0].blendEnable = true;
    pipelineInfo.colorBlend.blendAttachments[0].colorWriteMask = Gfx::SE_COLOR_COMPONENT_R_BIT | Gfx::SE_COLOR_COMPONENT_G_BIT | Gfx::SE_COLOR_COMPONENT_B_BIT | Gfx::SE_COLOR_COMPONENT_A_BIT;
    pipelineInfo.colorBlend.blendAttachments[0].srcColorBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA;
    pipelineInfo.colorBlend.blendAttachments[0].dstColorBlendFactor = Gfx::SE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineInfo.colorBlend.blendAttachments[0].alphaBlendOp = Gfx::SE_BLEND_OP_ADD;
    pipelineInfo.colorBlend.blendAttachments[0].srcAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineInfo.colorBlend.blendAttachments[0].dstAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_ZERO;
    pipelineInfo.colorBlend.blendAttachments[0].alphaBlendOp = Gfx::SE_BLEND_OP_ADD;
    pipelineInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    pipeline = graphics->createGraphicsPipeline(pipelineInfo);
}
TextPass::FontData& TextPass::getFontData(PFontAsset font)
{
    if(fontData.exists(font))
    {
        return fontData[font];
    } 
    const auto& fontGlyphs = font->getGlyphData();
    FontData& fd = fontData[font];
    Array<GlyphData> glyphData;
    Array<Gfx::PTexture> textures;
    glyphData.reserve(fontGlyphs.size());
    for(const auto& [key, value] : fontGlyphs)
    {
        fd.characterToGlyphIndex[key] = static_cast<uint32>(glyphData.size());
        GlyphData& gd = glyphData.add();
        gd.bearing = value.bearing;
        gd.size = value.size;
        gd.advance = value.advance;
        textures.add(value.texture);
    }
    fd.glyphDataSet = glyphData;
    
    fd.textureArraySet = textureArrayLayout->allocateDescriptorSet();
    fd.textureArraySet->updateTextureArray(0, textures);
    fd.textureArraySet->writeChanges();
    return fontData[font];
}