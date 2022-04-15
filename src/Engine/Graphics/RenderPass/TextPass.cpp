#include "TextPass.h"
#include "RenderGraph.h"
#include "Graphics/Graphics.h"
#include "Graphics/VertexShaderInput.h"

using namespace Seele;

TextPass::TextPass(Gfx::PGraphics graphics, Gfx::PViewport viewport, Gfx::PRenderTargetAttachment attachment) 
    : RenderPass(graphics, viewport)
    , renderTarget(attachment)
{
}

TextPass::~TextPass()
{
    
}

MainJob TextPass::beginFrame() 
{
    for(TextRender& render : passData.texts)
    {
        FontData& fontData = getFontData(render.font);
        TextResources& resources = textResources.add();
        Array<GlyphInstanceData> instanceData;
        float x = render.position.x;
        float y = render.position.y;
        for(uint32 c : render.text)
        {
            instanceData.add(GlyphInstanceData{
                .glyphIndex = fontData.characterToGlyphIndex[c],
                .position = Vector2(x, y)
            });
            x += (fontData.characterAdvance[c] >> 6) * render.scale;
        }
        VertexBufferCreateInfo vbInfo;
        vbInfo.numVertices = static_cast<uint32>(instanceData.size());
        vbInfo.vertexSize = sizeof(GlyphInstanceData);
        vbInfo.resourceData.data = reinterpret_cast<uint8*>(instanceData.data());
        vbInfo.resourceData.size = static_cast<uint32>(instanceData.size());
        resources.vertexBuffer = graphics->createVertexBuffer(vbInfo);

        resources.descriptorSet = descriptorLayout->allocateDescriptorSet();
        resources.descriptorSet->updateBuffer(0, projectionBuffer);
        resources.descriptorSet->updateSampler(1, glyphSampler);
        resources.descriptorSet->updateBuffer(2, fontData.structuredBuffer);
        resources.descriptorSet->writeChanges();

        resources.textureArraySet = descriptorLayout->allocateDescriptorSet();
        resources.textureArraySet->updateTextureArray(0, fontData.textures);
        
        resources.textureArraySet->writeChanges();
        resources.scale = render.scale;
    }
    co_return;
}

MainJob TextPass::render() 
{
    graphics->beginRenderPass(renderPass);
    Array<Gfx::PRenderCommand> commands;
    for(TextResources& resources : textResources)
    {
        Gfx::PRenderCommand command = graphics->createRenderCommand("TextPassCommand");
        command->setViewport(viewport);
        command->bindPipeline(pipeline);
        
        command->bindVertexBuffer({VertexInputStream(0, 0, resources.vertexBuffer)});
        
        command->pushConstants(pipelineLayout, Gfx::SE_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &resources.scale);
        command->draw(4, static_cast<uint32>(resources.vertexBuffer->getNumVertices()), 0, 0);
        commands.add(command);
    }
    graphics->executeCommands(commands);
    graphics->endRenderPass();
    co_return;
}

MainJob TextPass::endFrame() 
{
    co_return;
}

void TextPass::publishOutputs() 
{
}

void TextPass::createRenderPass() 
{
    depthAttachment = resources->requestRenderTarget("UIPASS_DEPTH");
    std::ifstream codeStream("./shaders/TextPass.slang", std::ios::ate);
    auto fileSize = codeStream.tellg();
    codeStream.seekg(0);
    Array<char> buffer(static_cast<uint32>(fileSize));
    codeStream.read(buffer.data(), fileSize);

    ShaderCreateInfo createInfo;
    createInfo.shaderCode.add(std::string(buffer.data()));
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
        .offset = offsetof(GlyphInstanceData, glyphIndex), 
        .vertexFormat = Gfx::SE_FORMAT_R32_UINT, 
        .attributeIndex = 0, 
        .stride = sizeof(GlyphInstanceData), 
        .bInstanced = 1
    });
    elements.add({
        .streamIndex = 0, 
        .offset = offsetof(GlyphInstanceData, position), 
        .vertexFormat = Gfx::SE_FORMAT_R32G32_SFLOAT, 
        .attributeIndex = 1,
        .stride = sizeof(GlyphInstanceData), 
        .bInstanced = 1
    });
    declaration = graphics->createVertexDeclaration(elements);

    descriptorLayout = graphics->createDescriptorLayout("TextDescriptor");
    descriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_SAMPLER);
    descriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, 128, Gfx::SE_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);
    descriptorLayout->create();

    textureArrayLayout = graphics->createDescriptorLayout("TextureArray");
    textureArrayLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 128, Gfx::SE_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);
    textureArrayLayout->create();

    Matrix4 projectionMatrix = glm::ortho(0.f, (float)viewport->getSizeX(), 0.f, (float)viewport->getSizeY());
    projectionBuffer = graphics->createUniformBuffer({
        .resourceData = {
            .size = sizeof(Matrix4),
            .data = (uint8*)&projectionMatrix,
        },
        .bDynamic = false,
    });

    glyphSampler = graphics->createSamplerState({
        .magFilter = Gfx::SE_FILTER_LINEAR,
        .minFilter = Gfx::SE_FILTER_LINEAR,
        .addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    });

    pipelineLayout = graphics->createPipelineLayout();
    pipelineLayout->addDescriptorLayout(0, descriptorLayout);
    pipelineLayout->addDescriptorLayout(1, textureArrayLayout);
    pipelineLayout->addPushConstants({
        .stageFlags = Gfx::SE_SHADER_STAGE_VERTEX_BIT, 
        .offset = 0, 
        .size = sizeof(float)});
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
    pipelineInfo.topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    pipeline = graphics->createGraphicsPipeline(pipelineInfo);
}
TextPass::FontData& TextPass::getFontData(PFontAsset font)
{
    if(fontData.exists(font))
    {
        return fontData[font];
    }
    const Map<uint32, FontAsset::Glyph>& fontGlyphs = font->getGlyphData();
    FontData& fd = fontData[font];
    Array<GlyphData> buffer;
    buffer.reserve(fontGlyphs.size());
    for(const auto& [key, value] : fontGlyphs)
    {
        fd.characterToGlyphIndex[key] = static_cast<uint32>(buffer.size());
        fd.characterAdvance[key] = value.advance;
        fd.textures.add(value.texture);
        GlyphData& gd = buffer.add();
        gd.bearing = value.bearing;
        gd.size = value.size;
    }
    StructuredBufferCreateInfo bufferInfo;
    bufferInfo.bDynamic = false;
    bufferInfo.resourceData.data = reinterpret_cast<uint8*>(buffer.data());
    bufferInfo.resourceData.size = static_cast<uint32>(buffer.size());
    fd.structuredBuffer = graphics->createStructuredBuffer(bufferInfo);
    return fontData[font];
}
