#include "UIPass.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/Command.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderTarget.h"
#include "RenderGraph.h"

using namespace Seele;

UIPass::UIPass(Gfx::PGraphics graphics, UI::PSystem system) : RenderPass(graphics), system(system) {
    glyphInstanceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{.name = "GlyphInstanceBuffer"});
    elementBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{.name = "RenderStyleElements"});
    textDescriptorLayout = graphics->createDescriptorLayout("pText");
    textDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = GLYPHINSTANCE_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    textDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = GLYPHSAMPLER_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    textDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TEXTURES_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 1024,
        .access = Gfx::SE_DESCRIPTOR_ACCESS_SAMPLE_BIT,
    });
    textPipelineLayout = graphics->createPipelineLayout("TextPipeline");
    textPipelineLayout->addDescriptorLayout(viewParamsLayout);
    textPipelineLayout->addDescriptorLayout(textDescriptorLayout);

    uiDescriptorLayout = graphics->createDescriptorLayout("pParams");
    uiDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = ELEMENT_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    uiDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = GLYPHSAMPLER_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    uiDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TEXTURES_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = 1024,
        .access = Gfx::SE_DESCRIPTOR_ACCESS_SAMPLE_BIT,
    });
    uiPipelineLayout = graphics->createPipelineLayout("UIPipeline");
    uiPipelineLayout->addDescriptorLayout(viewParamsLayout);
    uiPipelineLayout->addDescriptorLayout(uiDescriptorLayout);
    // todo:
}

UIPass::~UIPass() {}

void UIPass::beginFrame(const Component::Camera& cam, const Component::Transform& transform) {
    RenderPass::beginFrame(cam, transform);
    glyphs.clear();
    usedTextures.clear();
    elements.clear();
    for (auto& render : renderElements) {
        float x = render.position.x;
        float y = render.position.y;
        //todo: convert using actual tree depth
        uint32 textureIndex = std::numeric_limits<uint32>::max();
        if (render.backgroundTexture != nullptr) {
            textureIndex = (uint32)usedTextures.size();
            usedTextures.add(render.backgroundTexture);
        }
        elements.add(RenderElementStyle{
            .x = x,
            .y = y,
            .w = render.dimensions.x,
            .h = render.dimensions.y,
            .color = render.backgroundColor,
            .z = render.level / 100.f,
            .textureIndex = textureIndex,
        });
    }
    glyphInstanceBuffer->rotateBuffer(sizeof(GlyphInstanceData) * glyphs.size());
    glyphInstanceBuffer->updateContents(0, sizeof(GlyphInstanceData) * glyphs.size(), glyphs.data());
    glyphInstanceBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                         Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_VERTEX_SHADER_BIT);

    textDescriptorLayout->reset();
    textDescriptorSet = textDescriptorLayout->allocateDescriptorSet();
    textDescriptorSet->updateBuffer(GLYPHINSTANCE_NAME, 0, glyphInstanceBuffer);
    textDescriptorSet->updateSampler(GLYPHSAMPLER_NAME, 0, glyphSampler);
    for (uint32 i = 0; i < usedTextures.size(); ++i) {
        textDescriptorSet->updateTexture(TEXTURES_NAME, i, usedTextures[i]);
    }
    textDescriptorSet->writeChanges();

    elementBuffer->rotateBuffer(sizeof(RenderElementStyle) * elements.size());
    elementBuffer->updateContents(0, sizeof(RenderElementStyle) * elements.size(), elements.data());
    elementBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                   Gfx::SE_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    uiDescriptorLayout->reset();
    uiDescriptorSet = uiDescriptorLayout->allocateDescriptorSet();
    uiDescriptorSet->updateBuffer(ELEMENT_NAME, 0, elementBuffer);
    uiDescriptorSet->updateSampler(GLYPHSAMPLER_NAME, 0, glyphSampler);
    for (uint32 i = 0; i < usedTextures.size(); ++i) {
        uiDescriptorSet->updateTexture(TEXTURES_NAME, i, usedTextures[i]);
    }
    uiDescriptorSet->writeChanges();
}

void UIPass::render() {
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;
    Gfx::ORenderCommand command = graphics->createRenderCommand("TextPassCommand");
    command->setViewport(viewport);
    command->bindPipeline(uiPipeline);
    command->bindDescriptor({viewParamsSet, uiDescriptorSet});
    command->draw(4, (uint32)elements.size(), 0, 0);
    commands.add(std::move(command));
    graphics->executeCommands(std::move(commands));
    graphics->endRenderPass();
}

void UIPass::endFrame() {}

void UIPass::publishOutputs() {}

void UIPass::createRenderPass() {
    TextureCreateInfo depthBufferInfo = {
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getWidth(),
        .height = viewport->getHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };

    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment =
        Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    depthAttachment.clear.depthStencil.depth = 0;

    colorAttachment = Gfx::RenderTargetAttachment(viewport, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                  Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    colorAttachment.clear.color = {{1.0f, 1.0f, 1.0f, 1.0f}};

    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = {colorAttachment},
        .depthAttachment = depthAttachment,
    };
    Array<Gfx::SubPassDependency> dependency = {
        {
            .srcSubpass = ~0U,
            .dstSubpass = 0,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
    };
    renderPass = graphics->createRenderPass(std::move(layout), dependency, "TextPass");

    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .name = "TextVertex",
        .modules = {"TextPass"},
        .entryPoints = {{"vertexMain", "TextPass"}, {"fragmentMain", "TextPass"}},
        .rootSignature = textPipelineLayout,
    });
    textPipelineLayout->create();
    textVertexShader = graphics->createVertexShader({0});
    textFragmentShader = graphics->createFragmentShader({1});

    glyphSampler = graphics->createSampler({
        .magFilter = Gfx::SE_FILTER_LINEAR,
        .minFilter = Gfx::SE_FILTER_LINEAR,
        .addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    });

    textPipeline = graphics->createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo{
        .topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .vertexShader = textVertexShader,
        .fragmentShader = textFragmentShader,
        .renderPass = renderPass,
        .pipelineLayout = textPipelineLayout,
        .rasterizationState =
            {
                .cullMode = Gfx::SE_CULL_MODE_NONE,
            },
        .colorBlend =
            {

                .attachmentCount = 1,
                .blendAttachments = {{
                    .blendEnable = true,
                    .srcColorBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA,
                    .dstColorBlendFactor = Gfx::SE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    .srcAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    .dstAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = Gfx::SE_BLEND_OP_ADD,
                    .colorWriteMask = Gfx::SE_COLOR_COMPONENT_R_BIT | Gfx::SE_COLOR_COMPONENT_G_BIT | Gfx::SE_COLOR_COMPONENT_B_BIT |
                                      Gfx::SE_COLOR_COMPONENT_A_BIT,
                }},
            },
    });

    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .name = "UIVertex",
        .modules = {"UIPass"},
        .entryPoints = {{"vertexMain", "UIPass"}, {"fragmentMain", "UIPass"}},
        .rootSignature = uiPipelineLayout,
    });
    uiPipelineLayout->create();
    uiVertexShader = graphics->createVertexShader({0});
    uiFragmentShader = graphics->createFragmentShader({1});

    uiPipeline = graphics->createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo{
        .topology = Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        .vertexShader = uiVertexShader,
        .fragmentShader = uiFragmentShader,
        .renderPass = renderPass,
        .pipelineLayout = uiPipelineLayout,
        .rasterizationState =
            {
                .cullMode = Gfx::SE_CULL_MODE_NONE,
            },
        .colorBlend =
            {

                .attachmentCount = 1,
                .blendAttachments = {{
                    .blendEnable = true,
                    .srcColorBlendFactor = Gfx::SE_BLEND_FACTOR_SRC_ALPHA,
                    .dstColorBlendFactor = Gfx::SE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    .srcAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    .dstAlphaBlendFactor = Gfx::SE_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = Gfx::SE_BLEND_OP_ADD,
                    .colorWriteMask = Gfx::SE_COLOR_COMPONENT_R_BIT | Gfx::SE_COLOR_COMPONENT_G_BIT | Gfx::SE_COLOR_COMPONENT_B_BIT |
                                      Gfx::SE_COLOR_COMPONENT_A_BIT,
                }},
            },
    });
    system->style();
    system->layout(UVector2(viewport->getWidth(), viewport->getHeight()));
    renderElements = system->render();
}
