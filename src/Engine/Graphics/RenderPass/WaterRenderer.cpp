#include "WaterRenderer.h"
#include "Asset/AssetRegistry.h"
#include "Component/WaterTile.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"

using namespace Seele;

WaterRenderer::WaterRenderer(Gfx::PGraphics graphics, PScene scene, Gfx::PDescriptorLayout viewParamsLayout)
    : graphics(graphics), scene(scene) {
    skyBox = AssetRegistry::findTexture("", "skyboxsun5deg_tn")->getTexture().cast<Gfx::TextureCube>();
    logN = (int)log2(params.N);
    threadGroupsX = ceil(params.N / 8.0f);
    threadGroupsY = ceil(params.N / 8.0f);

    materialUniforms = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(MaterialParams),
                .data = (uint8*)&materialParams,
            },
        .dynamic = true,
        .name = "WaterMaterialParams",
    });
    materialUniforms->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                      Gfx::SE_ACCESS_UNIFORM_READ_BIT,
                                      Gfx::SE_PIPELINE_STAGE_MESH_SHADER_BIT_EXT | Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    paramsBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(ComputeParams),
            },
        .dynamic = true,
        .name = "WaterComputeParams",
    });
    paramsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_UNIFORM_READ_BIT,
                                  Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    initialSpectrumTextures = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = params.N,
        .height = params.N,
        .elements = 4,
        .useMip = true,
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
        .name = "InitialSpectrumTextures",
    });
    initialSpectrumTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                          Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                          Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    boyancyData = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R16_SFLOAT,
        .width = params.N,
        .height = params.N,
        .useMip = false,
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
        .name = "Bouyancy",
    });
    boyancyData->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    displacementTextures = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = params.N,
        .height = params.N,
        .elements = 4,
        .useMip = true,
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
        .name = "DisplacementTexture",
    });
    displacementTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                       Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                       Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    slopeTextures = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32_SFLOAT,
        .width = params.N,
        .height = params.N,
        .elements = 4,
        .useMip = true,
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
        .name = "SlopeTextures",
    });
    slopeTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    spectrumTextures = graphics->createTexture2D(TextureCreateInfo{
        .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
        .width = params.N,
        .height = params.N,
        .elements = 8,
        .useMip = true,
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
        .name = "SpectrumTextures",
    });
    spectrumTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                   Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                   Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    spectrumBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = spectrums.size() * sizeof(SpectrumParameters),
                .data = (uint8*)spectrums.data(),
            },
        .numElements = 8,
        .dynamic = true,
        .name = "Spectrums",
    });
    spectrumBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                    Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    linearRepeatSampler = graphics->createSampler(SamplerCreateInfo{
        .magFilter = Gfx::SE_FILTER_LINEAR,
        .minFilter = Gfx::SE_FILTER_LINEAR,
        .addressModeU = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = Gfx::SE_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = true,
        .maxAnisotropy = 16,
    });

    computeLayout = graphics->createDescriptorLayout("pParams");
    computeLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    });
    computeLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    computeLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 2,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    computeLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 3,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    computeLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 4,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    computeLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 5,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    });
    computeLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 6,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    computeLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 7,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    computeLayout->create();

    pipelineLayout = graphics->createPipelineLayout("WaterComputeLayout");
    pipelineLayout->addDescriptorLayout(computeLayout);

    ShaderCompilationInfo info = {
        .name = "WaterCompute",
        .modules = {"WaterCompute"},
        .entryPoints =
            {
                {"CS_InitializeSpectrum", "WaterCompute"},
                {"CS_PackSpectrumConjugate", "WaterCompute"},
                {"CS_UpdateSpectrumForFFT", "WaterCompute"},
                {"CS_HorizontalFFT", "WaterCompute"},
                {"CS_VerticalFFT", "WaterCompute"},
                {"CS_AssembleMaps", "WaterCompute"},
            },
        .rootSignature = pipelineLayout,
        .dumpIntermediate = true,
    };
    graphics->beginShaderCompilation(info);
    initSpectrumCS = graphics->createComputeShader({0});
    packSpectrumConjugateCS = graphics->createComputeShader({1});
    updateSpectrumForFFTCS = graphics->createComputeShader({2});
    horizontalFFTCS = graphics->createComputeShader({3});
    verticalFFTCS = graphics->createComputeShader({4});
    assembleMapsCS = graphics->createComputeShader({5});

    pipelineLayout->create();

    initSpectrum = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = initSpectrumCS,
        .pipelineLayout = pipelineLayout,
    });
    packSpectrumConjugate = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = packSpectrumConjugateCS,
        .pipelineLayout = pipelineLayout,
    });
    updateSpectrumForFFT = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = updateSpectrumForFFTCS,
        .pipelineLayout = pipelineLayout,
    });
    horizontalFFT = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = horizontalFFTCS,
        .pipelineLayout = pipelineLayout,
    });
    verticalFFT = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = verticalFFTCS,
        .pipelineLayout = pipelineLayout,
    });
    assembleMaps = graphics->createComputePipeline(Gfx::ComputePipelineCreateInfo{
        .computeShader = assembleMapsCS,
        .pipelineLayout = pipelineLayout,
    });

    materialLayout = graphics->createDescriptorLayout("pWaterMaterial");
    materialLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    });
    materialLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    materialLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 2,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    materialLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 3,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    materialLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 4,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLER,
    });
    materialLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 5,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    materialLayout->create();
    waterLayout = graphics->createPipelineLayout("WaterLayout");
    waterLayout->addDescriptorLayout(viewParamsLayout);
    waterLayout->addDescriptorLayout(materialLayout);

    ShaderCompilationInfo createInfo = {
        .name = "WaterTiles",
        .modules = {"WaterTask", "WaterMesh", "WaterPass"},
        .entryPoints =
            {
                {"taskMain", "WaterTask"},
                {"meshMain", "WaterMesh"},
                {"fragmentMain", "WaterPass"},
            },
        .rootSignature = waterLayout,
        .dumpIntermediate = true,
    };
    graphics->beginShaderCompilation(createInfo);
    waterTask = graphics->createTaskShader({0});
    waterMesh = graphics->createMeshShader({1});
    waterFragment = graphics->createFragmentShader({2});

    waterLayout->create();

    displaySpectrums[0] = {
        .scale = 0.01f,
        .windSpeed = 2,
        .windDirection = 22,
        .fetch = 100000,
        .spreadBlend = 0.642,
        .swell = 1,
        .peakEnhancement = 1,
        .shortWavesFade = 0.25f,
    };
    displaySpectrums[1] = {
        .scale = 0.07f,
        .windSpeed = 2,
        .windDirection = 59,
        .fetch = 1000,
        .spreadBlend = 0,
        .swell = 1,
        .peakEnhancement = 1,
        .shortWavesFade = 0.01f,
    };
    displaySpectrums[2] = {
        .scale = 0.25f,
        .windSpeed = 20,
        .windDirection = 97,
        .fetch = 100000000,
        .spreadBlend = 0.14f,
        .swell = 1,
        .peakEnhancement = 1,
        .shortWavesFade = 0.5f,
    };
    displaySpectrums[3] = {
        .scale = 0.25f,
        .windSpeed = 20,
        .windDirection = 67,
        .fetch = 1000000,
        .spreadBlend = 0.47f,
        .swell = 1,
        .peakEnhancement = 1,
        .shortWavesFade = 0.5f,
    };
    displaySpectrums[4] = {
        .scale = 0.15f,
        .windSpeed = 5,
        .windDirection = 105,
        .fetch = 1000000,
        .spreadBlend = 0.2f,
        .swell = 1,
        .peakEnhancement = 1,
        .shortWavesFade = 0.5f,
    };
    displaySpectrums[5] = {
        .scale = 0.1f,
        .windSpeed = 1,
        .windDirection = 19,
        .fetch = 10000,
        .spreadBlend = 0.298f,
        .swell = 0.695f,
        .peakEnhancement = 1,
        .shortWavesFade = 0.5f,
    };
    displaySpectrums[6] = {
        .scale = 1.0f,
        .windSpeed = 1,
        .windDirection = 209,
        .fetch = 200000,
        .spreadBlend = 0.56f,
        .swell = 1,
        .peakEnhancement = 1,
        .shortWavesFade = 0.0001f,
    };
    displaySpectrums[7] = {
        .scale = 0.25f,
        .windSpeed = 1,
        .windDirection = 0,
        .fetch = 1000,
        .spreadBlend = 0,
        .swell = 0,
        .peakEnhancement = 1,
        .shortWavesFade = 0.0001f,
    };
}

WaterRenderer::~WaterRenderer() {}

void WaterRenderer::beginFrame() {
    struct WaterTile {
        IVector2 offset;
        float extent;
        float height;
    };
    Array<WaterTile> payloads;
    scene->view<Component::WaterTile>([&](Component::WaterTile& tile) {
        payloads.add(WaterTile{
            .offset = Vector2(tile.location),
            .extent = Component::WaterTile::DIMENSIONS,
            .height = tile.height,
        });
    });
    waterTiles = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = payloads.size() * sizeof(WaterTile),
                .data = (uint8*)payloads.data(),
            },
        .numElements = payloads.size(),
        .name = "WaterTiles",
    });
    waterTiles->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT);

    displacementTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_SHADER_STAGE_MESH_BIT_EXT,
                                       Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    slopeTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_SHADER_STAGE_MESH_BIT_EXT,
                                Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    boyancyData->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_SHADER_STAGE_MESH_BIT_EXT,
                              Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    updateFFTDescriptor();
    Gfx::OComputeCommand updateCommand = graphics->createComputeCommand("WaterUpdate");
    updateCommand->bindPipeline(updateSpectrumForFFT);
    updateCommand->bindDescriptor(computeSet);
    updateCommand->dispatch(threadGroupsX, threadGroupsY, 1);
    graphics->executeCommands(std::move(updateCommand));

    spectrumTextures->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                      Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                      Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    Gfx::OComputeCommand horizontalCommand = graphics->createComputeCommand("HorizontalFFT");
    horizontalCommand->bindPipeline(horizontalFFT);
    horizontalCommand->bindDescriptor(computeSet);
    horizontalCommand->dispatch(1, params.N, 1);
    graphics->executeCommands(std::move(horizontalCommand));

    spectrumTextures->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    Gfx::OComputeCommand verticalCommand = graphics->createComputeCommand("VerticalFFT");
    verticalCommand->bindPipeline(verticalFFT);
    verticalCommand->bindDescriptor(computeSet);
    verticalCommand->dispatch(1, params.N, 1);
    graphics->executeCommands(std::move(verticalCommand));

    spectrumTextures->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    Gfx::OComputeCommand assembleCommand = graphics->createComputeCommand("AssembleCommand");
    assembleCommand->bindPipeline(assembleMaps);
    assembleCommand->bindDescriptor(computeSet);
    assembleCommand->dispatch(threadGroupsX, threadGroupsY, 1);
    graphics->executeCommands(std::move(assembleCommand));

    // transition for mipmap gen
    displacementTextures->changeLayout(
        Gfx::SE_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_TRANSFER_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);

    slopeTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                Gfx::SE_ACCESS_TRANSFER_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);

    displacementTextures->generateMipmaps();
    slopeTextures->generateMipmaps();

    displacementTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                       Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                       Gfx::SE_SHADER_STAGE_MESH_BIT_EXT);

    slopeTextures->changeLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_SHADER_STAGE_MESH_BIT_EXT);

    boyancyData->changeLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                              Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_SHADER_STAGE_MESH_BIT_EXT);

    updateMaterialDescriptor();
}

Gfx::ORenderCommand WaterRenderer::render(Gfx::PDescriptorSet viewParamsSet) {
    Gfx::ORenderCommand waterCommand = graphics->createRenderCommand("WaterRender");
    waterCommand->setViewport(viewport);
    waterCommand->bindPipeline(waterPipeline);
    waterCommand->bindDescriptor({viewParamsSet, materialSet});
    waterCommand->drawMesh(waterTiles->getNumElements(), 4, 1);
    return waterCommand;
}

void WaterRenderer::setViewport(Gfx::PViewport _viewport, Gfx::PRenderPass renderPass) {
    viewport = _viewport;
    updateFFTDescriptor();
    Gfx::MeshPipelineCreateInfo pipelineInfo = {
        .taskShader = waterTask,
        .meshShader = waterMesh,
        .fragmentShader = waterFragment,
        .renderPass = renderPass,
        .pipelineLayout = waterLayout,
        .multisampleState =
            {
                .samples = viewport->getSamples(),
            },
        .rasterizationState =
            {
                .cullMode = Gfx::SE_CULL_MODE_BACK_BIT,
            },
        .colorBlend =
            {
                .attachmentCount = 1,
                .blendAttachments =
                    {
                        Gfx::ColorBlendState::BlendAttachment{
                            .blendEnable = false,
                        },
                    },
            },
    };
    waterPipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
    Gfx::OComputeCommand initCmd = graphics->createComputeCommand("InitialSpectrumCompute");
    initCmd->bindPipeline(initSpectrum);
    initCmd->bindDescriptor(computeSet);
    initCmd->dispatch(threadGroupsX, threadGroupsY, 1);
    graphics->executeCommands(std::move(initCmd));

    initialSpectrumTextures->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                             Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    Gfx::OComputeCommand packCmd = graphics->createComputeCommand("PackConjugate");
    packCmd->bindPipeline(packSpectrumConjugate);
    packCmd->bindDescriptor(computeSet);
    packCmd->dispatch(threadGroupsX, threadGroupsY, 1);
    graphics->executeCommands(std::move(packCmd));

    initialSpectrumTextures->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                             Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

float WaterRenderer::jonswapAlpha(float fetch, float windSpeed) const {
    return 0.076f * pow(params.gravity * fetch / windSpeed / windSpeed, -0.22f);
}

float WaterRenderer::jonswapPeakFrequency(float fetch, float windSpeed) const {
    return 22 * pow(windSpeed * fetch / params.gravity / params.gravity, -0.33f);
}

void WaterRenderer::updateFFTDescriptor() {
    for (uint32 i = 0; i < displaySpectrums.size(); ++i) {
        spectrums[i] = {
            .scale = displaySpectrums[i].scale,
            .angle = displaySpectrums[i].windDirection / 180 * 3.1415f,
            .spreadBlend = displaySpectrums[i].spreadBlend,
            .swell = std::clamp(displaySpectrums[i].swell, 0.01f, 1.0f),
            .alpha = jonswapAlpha(displaySpectrums[i].fetch, displaySpectrums[i].windSpeed),
            .peakOmega = jonswapPeakFrequency(displaySpectrums[i].fetch, displaySpectrums[i].windSpeed),
            .gamma = displaySpectrums[i].peakEnhancement,
            .shortWavesFade = displaySpectrums[i].shortWavesFade,
        };
    }
    spectrumBuffer->updateContents(0, sizeof(SpectrumParameters) * spectrums.size(), spectrums.data());
    spectrumBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                    Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    params.deltaTime = Gfx::getCurrentFrameDelta();
    params.frameTime = Gfx::getCurrentFrameTime();
    paramsBuffer->updateContents(0, sizeof(ComputeParams), &params);
    paramsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_UNIFORM_READ_BIT,
                                  Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    computeSet = computeLayout->allocateDescriptorSet();
    computeSet->updateBuffer(0, paramsBuffer);
    computeSet->updateTexture(1, 0, Gfx::PTexture2D(spectrumTextures));
    computeSet->updateTexture(2, 0, Gfx::PTexture2D(initialSpectrumTextures));
    computeSet->updateTexture(3, 0, Gfx::PTexture2D(displacementTextures));
    computeSet->updateTexture(4, 0, Gfx::PTexture2D(slopeTextures));
    computeSet->updateTexture(5, 0, Gfx::PTexture2D(boyancyData));
    computeSet->updateBuffer(6, 0, spectrumBuffer);
    computeSet->updateSampler(7, 0, linearRepeatSampler);
    computeSet->writeChanges();
}

void WaterRenderer::updateMaterialDescriptor() {
    materialUniforms->updateContents(0, sizeof(MaterialParams), &materialParams);
    materialUniforms->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                      Gfx::SE_ACCESS_UNIFORM_READ_BIT,
                                      Gfx::SE_PIPELINE_STAGE_MESH_SHADER_BIT_EXT | Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    materialSet = materialLayout->allocateDescriptorSet();
    materialSet->updateBuffer(0, materialUniforms);
    materialSet->updateTexture(1, Gfx::PTexture2D(displacementTextures));
    materialSet->updateTexture(2, Gfx::PTexture2D(slopeTextures));
    materialSet->updateTexture(3, Gfx::PTextureCube(skyBox));
    materialSet->updateSampler(4, linearRepeatSampler);
    materialSet->updateBuffer(5, waterTiles);
    materialSet->writeChanges();
}
