#include "ToneMappingPass.h"

using namespace Seele;

ToneMappingPass::ToneMappingPass(Gfx::PGraphics graphics) : RenderPass(graphics) {
    layout = graphics->createDescriptorLayout("ToneMappingDescriptor");
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "offset",
        .uniformLength = sizeof(Vector4),
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "slope",
        .uniformLength = sizeof(Vector4),
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "power",
        .uniformLength = sizeof(Vector4),
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "sat",
        .uniformLength = sizeof(float),
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "hdrInputTexture",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    layout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "hdrSampler",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    layout->create();
    pipelineLayout = graphics->createPipelineLayout("ToneMappingLayout");
    pipelineLayout->addDescriptorLayout(layout);
    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .name = "ToneMapping",
        .modules = {"FullScreenQuad", "ToneMapping"},
        .entryPoints = {{"quadMain", "FullScreenQuad"}, {"toneMapping", "ToneMapping"}},
        .rootSignature = pipelineLayout,
    });
    pipelineLayout->create();
    vert = graphics->createVertexShader({0});
    frag = graphics->createFragmentShader({1});
    sampler = graphics->createSampler({});
}

ToneMappingPass::~ToneMappingPass() {}

void ToneMappingPass::beginFrame(const Component::Camera& cam) { RenderPass::beginFrame(cam); }

void ToneMappingPass::render() {
    hdrInputTexture.getTexture()->changeLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                               Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                               Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    layout->reset();
    set = layout->allocateDescriptorSet();
    set->updateTexture("hdrInputTexture", 0, hdrInputTexture.getTexture());
    set->updateSampler("hdrSampler", 0, sampler);
    set->writeChanges();
    graphics->beginRenderPass(renderPass);
    Gfx::ORenderCommand command = graphics->createRenderCommand("ToneMapping");
    command->setViewport(viewport);
    command->bindPipeline(pipeline);
    command->bindDescriptor({set});
    command->draw(3, 1, 0, 0);
    graphics->executeCommands(std::move(command));
    graphics->endRenderPass();
}

void ToneMappingPass::endFrame() {}

void ToneMappingPass::publishOutputs() {
    colorAttachment = Gfx::RenderTargetAttachment(viewport, Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                  Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("TONEMAPPING_COLOR", colorAttachment);
}

void ToneMappingPass::createRenderPass() {
    hdrInputTexture = resources->requestRenderTarget("BASEPASS_COLOR");
    Gfx::RenderTargetLayout targetLayout = Gfx::RenderTargetLayout{
        .colorAttachments = {colorAttachment},
    };
    Array<Gfx::SubPassDependency> dependency = {
        {
            .srcSubpass = ~0U,
            .dstSubpass = 0,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },
    };
    renderPass = graphics->createRenderPass(targetLayout, dependency, viewport, "ToneMappingPass");
    pipeline = graphics->createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo{
        .vertexShader = vert,
        .fragmentShader = frag,
        .renderPass = renderPass,
        .pipelineLayout = pipelineLayout,
        .colorBlend =
            {
                .attachmentCount = 1,
            },
    });
}
