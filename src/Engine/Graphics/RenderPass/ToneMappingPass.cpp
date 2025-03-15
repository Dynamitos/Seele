#include "ToneMappingPass.h"

using namespace Seele;

ToneMappingPass::ToneMappingPass(Gfx::PGraphics graphics) : RenderPass(graphics) {
    tonemappingLayout = graphics->createDescriptorLayout("ToneMappingDescriptor");
    tonemappingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "offset",
        .uniformLength = sizeof(Vector4),
    });
    tonemappingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "slope",
        .uniformLength = sizeof(Vector4),
    });
    tonemappingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "power",
        .uniformLength = sizeof(Vector4),
    });
    tonemappingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "sat",
        .uniformLength = sizeof(float),
    });
    tonemappingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "hdrInputTexture",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    tonemappingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "hdrSampler",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    tonemappingLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "averageLuminance",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    tonemappingLayout->create();
    tonemappingPipelineLayout = graphics->createPipelineLayout("ToneMappingLayout");
    tonemappingPipelineLayout->addDescriptorLayout(tonemappingLayout);

    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .name = "ToneMapping",
        .modules = {"FullScreenQuad", "ToneMapping"},
        .entryPoints =
            {
                {"quadMain", "FullScreenQuad"},
                {"toneMapping", "ToneMapping"},
            },
        .rootSignature = tonemappingPipelineLayout,
    });
    tonemappingPipelineLayout->create();
    vert = graphics->createVertexShader({0});
    frag = graphics->createFragmentShader({1});

    histogramLayout = graphics->createDescriptorLayout("pHistogramParams");
    histogramLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "minLogLum",
        .uniformLength = sizeof(float),
    });
    histogramLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "inverseLogLumRange",
        .uniformLength = sizeof(float),
    });
    histogramLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "timeCoeff",
        .uniformLength = sizeof(float),
    });
    histogramLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "numPixels",
        .uniformLength = sizeof(uint32),
    });
    histogramLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "hdrImage",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    histogramLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "histogram",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    histogramLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "averageLuminance",
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    histogramLayout->create();
    histogramPipelineLayout = graphics->createPipelineLayout("HistogramPipelineLayout");
    histogramPipelineLayout->addDescriptorLayout(histogramLayout);

    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .name = "Exposure",
        .modules = {"Exposure"},
        .entryPoints =
            {
                {"histogram", "Exposure"},
                {"exposure", "Exposure"},
            },
        .rootSignature = histogramPipelineLayout,
    });
    histogramPipelineLayout->create();
    histogramShader = graphics->createComputeShader({0});
    histogramPipeline = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = histogramShader,
        .pipelineLayout = histogramPipelineLayout,
    });
    exposureShader = graphics->createComputeShader({1});
    exposurePipeline = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = exposureShader,
        .pipelineLayout = histogramPipelineLayout,
    });
    histogramBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32) * 256,
            },
        .name = "HistogramBuffer",
    });
    luminanceBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(uint32),
            },
        .name = "LuminanceBuffer",
    });
    sampler = graphics->createSampler({});
}

ToneMappingPass::~ToneMappingPass() {}

void ToneMappingPass::beginFrame(const Component::Camera& cam) { RenderPass::beginFrame(cam); }

void ToneMappingPass::render() {
    hdrInputTexture.getTexture()->changeLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                               Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                               Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    histogramLayout->reset();
    Gfx::PDescriptorSet histogramSet = histogramLayout->allocateDescriptorSet();
    histogramSet->updateConstants("minLogLum", 0, &minLogLum);
    histogramSet->updateConstants("inverseLogLumRange", 0, &inverseLogLumRange);
    histogramSet->updateConstants("timeCoeff", 0, &timeCoeff);
    histogramSet->updateConstants("numPixels", 0, &numPixels);
    histogramSet->updateTexture("hdrImage", 0, hdrInputTexture.getTexture());
    histogramSet->updateBuffer("histogram", 0, histogramBuffer);
    histogramSet->updateBuffer("averageLuminance", 0, luminanceBuffer);
    histogramSet->writeChanges();

    {
        Gfx::OComputeCommand computeCommand = graphics->createComputeCommand("HistogramCommand");
        computeCommand->bindPipeline(histogramPipeline);
        computeCommand->bindDescriptor({histogramSet});
        computeCommand->dispatch(threadGroups.x, threadGroups.y, 1);
        graphics->executeCommands(std::move(computeCommand));
    }
    histogramBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                     Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    {
        Gfx::OComputeCommand computeCommand = graphics->createComputeCommand("ExposureCommand");
        computeCommand->bindPipeline(exposurePipeline);
        computeCommand->bindDescriptor({histogramSet});
        computeCommand->dispatch(1, 1, 1);
        graphics->executeCommands(std::move(computeCommand));
    }
    luminanceBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                     Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                     Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    tonemappingLayout->reset();
    Gfx::PDescriptorSet tonemappingSet = tonemappingLayout->allocateDescriptorSet();
    tonemappingSet->updateConstants("offset", 0, &offset);
    tonemappingSet->updateConstants("slope", 0, &slope);
    tonemappingSet->updateConstants("power", 0, &power);
    tonemappingSet->updateConstants("sat", 0, &sat);
    tonemappingSet->updateTexture("hdrInputTexture", 0, hdrInputTexture.getTexture());
    tonemappingSet->updateSampler("hdrSampler", 0, sampler);
    tonemappingSet->updateBuffer("averageLuminance", 0, luminanceBuffer);
    tonemappingSet->writeChanges();
    graphics->beginRenderPass(renderPass);
    Gfx::ORenderCommand command = graphics->createRenderCommand("ToneMapping");
    command->setViewport(viewport);
    command->bindPipeline(pipeline);
    command->bindDescriptor({tonemappingSet});
    command->draw(3, 1, 0, 0);
    graphics->executeCommands(std::move(command));
    graphics->endRenderPass();
}

void ToneMappingPass::endFrame() {}

void ToneMappingPass::publishOutputs() {
    numPixels = viewport->getWidth() * viewport->getHeight();
    threadGroups = UVector2((viewport->getWidth() + 15) / 16, (viewport->getHeight() + 15) / 16);
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
        .pipelineLayout = tonemappingPipelineLayout,
        .colorBlend =
            {
                .attachmentCount = 1,
            },
    });
}
