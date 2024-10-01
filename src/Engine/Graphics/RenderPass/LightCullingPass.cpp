#include "LightCullingPass.h"
#include "Actor/CameraActor.h"
#include "Component/Camera.h"
#include "Graphics/Command.h"
#include "Graphics/Graphics.h"
#include "RenderGraph.h"
#include "Scene/Scene.h"

using namespace Seele;

LightCullingPass::LightCullingPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics, scene) {}

LightCullingPass::~LightCullingPass() {}

void LightCullingPass::beginFrame(const Component::Camera& cam) {
    RenderPass::beginFrame(cam);

    oLightIndexCounter->pipelineBarrier(Gfx::SE_ACCESS_MEMORY_WRITE_BIT | Gfx::SE_ACCESS_MEMORY_READ_BIT,
                                        Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, Gfx::SE_ACCESS_MEMORY_WRITE_BIT,
                                        Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    tLightIndexCounter->pipelineBarrier(Gfx::SE_ACCESS_MEMORY_WRITE_BIT | Gfx::SE_ACCESS_MEMORY_READ_BIT,
                                        Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, Gfx::SE_ACCESS_MEMORY_WRITE_BIT,
                                        Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    uint32 reset = 0;
    oLightIndexCounter->rotateBuffer(sizeof(uint32));
    oLightIndexCounter->updateContents(0, sizeof(uint32), &reset);
    tLightIndexCounter->rotateBuffer(sizeof(uint32));
    tLightIndexCounter->updateContents(0, sizeof(uint32), &reset);
    oLightIndexCounter->pipelineBarrier(Gfx::SE_ACCESS_MEMORY_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                        Gfx::SE_ACCESS_MEMORY_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    tLightIndexCounter->pipelineBarrier(Gfx::SE_ACCESS_MEMORY_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                        Gfx::SE_ACCESS_MEMORY_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    cullingDescriptorLayout->reset();
    cullingDescriptorSet = cullingDescriptorLayout->allocateDescriptorSet();
}

void LightCullingPass::render() {
    query->beginQuery();
    timestamps->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "LightCullBegin");
    oLightGrid->pipelineBarrier(Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    tLightGrid->pipelineBarrier(Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    cullingDescriptorSet->updateTexture(0, 0, depthAttachment);
    cullingDescriptorSet->updateBuffer(1, 0, oLightIndexCounter);
    cullingDescriptorSet->updateBuffer(2, 0, tLightIndexCounter);
    cullingDescriptorSet->updateBuffer(3, 0, oLightIndexList);
    cullingDescriptorSet->updateBuffer(4, 0, tLightIndexList);
    cullingDescriptorSet->updateTexture(5, 0, oLightGrid);
    cullingDescriptorSet->updateTexture(6, 0, tLightGrid);
    cullingDescriptorSet->writeChanges();
    Gfx::OComputeCommand computeCommand = graphics->createComputeCommand("CullingCommand");
    if (getGlobals().useLightCulling) {
        computeCommand->bindPipeline(cullingEnabledPipeline);
    } else {
        computeCommand->bindPipeline(cullingPipeline);
    }
    computeCommand->bindDescriptor({viewParamsSet, dispatchParamsSet, cullingDescriptorSet, lightEnv->getDescriptorSet()});
    computeCommand->dispatch(dispatchParams.numThreadGroups.x, dispatchParams.numThreadGroups.y, dispatchParams.numThreadGroups.z);
    Array<Gfx::OComputeCommand> commands;
    commands.add(std::move(computeCommand));
    // std::cout << "Execute" << std::endl;
    graphics->executeCommands(std::move(commands));
    timestamps->write(Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, "LightCullEnd");
    query->endQuery();
    oLightIndexList->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    tLightIndexList->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    oLightGrid->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    tLightGrid->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void LightCullingPass::endFrame() {}

void LightCullingPass::publishOutputs() {
    setupFrustums();
    uint32_t viewportWidth = viewport->getWidth();
    uint32_t viewportHeight = viewport->getHeight();
    glm::uvec3 numThreadGroups = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
    dispatchParams.numThreadGroups = numThreadGroups;
    dispatchParams.numThreads = numThreadGroups * glm::uvec3(BLOCK_SIZE, BLOCK_SIZE, 1);
    dispatchParamsBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(DispatchParams),
                .data = (uint8*)&dispatchParams,
                .owner = Gfx::QueueType::COMPUTE,
            },
        .name = "DispatchParams",
    });
    dispatchParamsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                          Gfx::SE_ACCESS_UNIFORM_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    dispatchParamsSet = dispatchParamsLayout->allocateDescriptorSet();
    dispatchParamsSet->updateBuffer(0, 0, dispatchParamsBuffer);
    dispatchParamsSet->updateBuffer(1, 0, frustumBuffer);
    dispatchParamsSet->writeChanges();

    cullingDescriptorLayout = graphics->createDescriptorLayout("pCullingParams");

    // DepthTexture
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    // o_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_WRITE_BIT});
    // t_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_WRITE_BIT});
    // o_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 3, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_WRITE_BIT});
    // t_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 4, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_WRITE_BIT});
    // o_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 5, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_WRITE_BIT});
    // t_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 6, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_WRITE_BIT});

    cullingDescriptorLayout->create();

    lightEnv = scene->getLightEnvironment();

    {
        cullingLayout = graphics->createPipelineLayout("CullingLayout");
        cullingLayout->addDescriptorLayout(viewParamsLayout);
        cullingLayout->addDescriptorLayout(dispatchParamsLayout);
        cullingLayout->addDescriptorLayout(cullingDescriptorLayout);
        cullingLayout->addDescriptorLayout(lightEnv->getDescriptorLayout());

        ShaderCompilationInfo createInfo = {
            .name = "Culling",
            .modules = {"LightCulling"},
            .entryPoints = {{"cullLights", "LightCulling"}},
            .rootSignature = cullingLayout,
        };
        graphics->beginShaderCompilation(createInfo);
        cullingShader = graphics->createComputeShader({0});
        cullingLayout->create();

        Gfx::ComputePipelineCreateInfo pipelineInfo;
        pipelineInfo.computeShader = cullingShader;
        pipelineInfo.pipelineLayout = std::move(cullingLayout);
        cullingPipeline = graphics->createComputePipeline(std::move(pipelineInfo));
    }

    {
        cullingEnableLayout = graphics->createPipelineLayout("CullingEnabledLayout");
        cullingEnableLayout->addDescriptorLayout(viewParamsLayout);
        cullingEnableLayout->addDescriptorLayout(dispatchParamsLayout);
        cullingEnableLayout->addDescriptorLayout(cullingDescriptorLayout);
        cullingEnableLayout->addDescriptorLayout(lightEnv->getDescriptorLayout());

        ShaderCompilationInfo createInfo = {
            .name = "Culling",
            .modules = {"LightCulling"},
            .entryPoints = {{"cullLights", "LightCulling"}},
            .rootSignature = cullingEnableLayout,
        };
        createInfo.defines["LIGHT_CULLING"] = "1";
        graphics->beginShaderCompilation(createInfo);
        cullingEnabledShader = graphics->createComputeShader({0});
        cullingEnableLayout->create();

        Gfx::ComputePipelineCreateInfo pipelineInfo;
        pipelineInfo.computeShader = cullingShader;
        pipelineInfo.pipelineLayout = std::move(cullingEnableLayout);
        cullingEnabledPipeline = graphics->createComputePipeline(std::move(pipelineInfo));
    }

    uint32 counterReset = 0;
    ShaderBufferCreateInfo structInfo = {
        .sourceData =
            {
                .size = sizeof(uint32),
                .data = (uint8*)&counterReset,
                .owner = Gfx::QueueType::COMPUTE,
            },
        .numElements = 1,
        .name = "oLightIndexCounter",
    };
    oLightIndexCounter = graphics->createShaderBuffer(structInfo);
    structInfo.name = "tLightIndexCounter";
    tLightIndexCounter = graphics->createShaderBuffer(structInfo);
    structInfo = {
        .sourceData =
            {
                .size = (uint32)sizeof(uint32) * dispatchParams.numThreadGroups.x * dispatchParams.numThreadGroups.y *
                        dispatchParams.numThreadGroups.z * 8192,
                .data = nullptr,
                .owner = Gfx::QueueType::COMPUTE,
            },
        .name = "oLightIndexList",
    };
    oLightIndexList = graphics->createShaderBuffer(structInfo);
    structInfo.name = "tLightIndexList";
    tLightIndexList = graphics->createShaderBuffer(structInfo);
    resources->registerBufferOutput("LIGHTCULLING_OLIGHTLIST", oLightIndexList);
    resources->registerBufferOutput("LIGHTCULLING_TLIGHTLIST", tLightIndexList);

    TextureCreateInfo textureInfo = {
        .format = Gfx::SE_FORMAT_R32G32_UINT,
        .width = dispatchParams.numThreadGroups.x,
        .height = dispatchParams.numThreadGroups.y,
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
    };
    oLightGrid = graphics->createTexture2D(textureInfo);
    tLightGrid = graphics->createTexture2D(textureInfo);
    oLightGrid->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    tLightGrid->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_NONE, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    resources->registerTextureOutput("LIGHTCULLING_OLIGHTGRID", Gfx::PTexture2D(oLightGrid));
    resources->registerTextureOutput("LIGHTCULLING_TLIGHTGRID", Gfx::PTexture2D(tLightGrid));

    query = graphics->createPipelineStatisticsQuery("LightCullPipelineStatistics");
    resources->registerQueryOutput("LIGHTCULL_QUERY", query);
}

void LightCullingPass::createRenderPass() {
    timestamps = resources->requestTimestampQuery("TIMESTAMPS");
    depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH").getTexture();
}

void LightCullingPass::setupFrustums() {
    uint32_t viewportWidth = viewport->getWidth();
    uint32_t viewportHeight = viewport->getHeight();

    glm::uvec3 numThreads = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
    glm::uvec3 numThreadGroups = glm::ceil(glm::vec3(numThreads.x / (float)BLOCK_SIZE, numThreads.y / (float)BLOCK_SIZE, 1));

    RenderPass::beginFrame(Component::Camera());
    viewParamsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                      Gfx::SE_ACCESS_UNIFORM_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    dispatchParams.numThreads = numThreads;
    dispatchParams.numThreadGroups = numThreadGroups;

    dispatchParamsLayout = graphics->createDescriptorLayout("pDispatchParams");
    dispatchParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    });
    dispatchParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .access = Gfx::SE_DESCRIPTOR_ACCESS_WRITE_ONLY_BIT,
    });
    dispatchParamsLayout->create();
    frustumLayout = graphics->createPipelineLayout("FrustumLayout");
    frustumLayout->addDescriptorLayout(viewParamsLayout);
    frustumLayout->addDescriptorLayout(dispatchParamsLayout);
    ShaderCompilationInfo createInfo = {
        .name = "Frustum",
        .modules = {"ComputeFrustums"},
        .entryPoints = {{"computeFrustums", "ComputeFrustums"}},
        .rootSignature = frustumLayout,
    };
    graphics->beginShaderCompilation(createInfo);
    frustumShader = graphics->createComputeShader({0});
    // Have to compile shader before finalizing layout as parameters get mapped later
    frustumLayout->create();
    dispatchParamsLayout->create();

    Gfx::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = frustumShader;
    pipelineInfo.pipelineLayout = frustumLayout;
    frustumPipeline = graphics->createComputePipeline(pipelineInfo);

    Gfx::OUniformBuffer frustumDispatchParamsBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(DispatchParams),
                .data = (uint8*)&dispatchParams,
                .owner = Gfx::QueueType::COMPUTE,
            },
        .name = "FrustumDispatch",
    });
    frustumDispatchParamsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                                 Gfx::SE_ACCESS_UNIFORM_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    frustumBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = sizeof(Frustum) * numThreads.x * numThreads.y * numThreads.z,
                .data = nullptr,
                .owner = Gfx::QueueType::COMPUTE,
            },
        .numElements = numThreads.x * numThreads.y * numThreads.z,
        .name = "FrustumBuffer",
    });
    frustumBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                   Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    Gfx::PDescriptorSet dispatchParamsSet = dispatchParamsLayout->allocateDescriptorSet();
    dispatchParamsSet->updateBuffer(0, 0, frustumDispatchParamsBuffer);
    dispatchParamsSet->updateBuffer(1, 0, frustumBuffer);
    dispatchParamsSet->writeChanges();

    Gfx::OComputeCommand command = graphics->createComputeCommand("FrustumCommand");
    command->bindPipeline(frustumPipeline);
    command->bindDescriptor({viewParamsSet, dispatchParamsSet});
    command->dispatch(numThreadGroups.x, numThreadGroups.y, numThreadGroups.z);
    Array<Gfx::OComputeCommand> commands;
    commands.add(std::move(command));
    graphics->executeCommands(std::move(commands));
    frustumBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                   Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
