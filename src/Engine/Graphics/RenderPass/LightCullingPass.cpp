#include "LightCullingPass.h"
#include "Actor/CameraActor.h"
#include "Component/Camera.h"
#include "Component/Transform.h"
#include "Graphics/Command.h"
#include "Graphics/Graphics.h"
#include "Graphics/Pipeline.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Scene/Scene.h"

using namespace Seele;

LightCullingPass::LightCullingPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics), scene(scene) {}

LightCullingPass::~LightCullingPass() {}

void LightCullingPass::beginFrame(const Component::Camera& cam, const Component::Transform& transform) {
    updateViewParameters(cam, transform);
    viewParamsSet = createViewParamsSet();

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
    graphics->beginDebugRegion("LightCulling");
    query->beginQuery();
    timestamps->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "LightCullBegin");
    oLightGrid->pipelineBarrier(Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    tLightGrid->pipelineBarrier(Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    cullingDescriptorSet->updateTexture(DEPTHATTACHMENT_NAME, 0, depthAttachment);
    cullingDescriptorSet->updateBuffer(OLIGHTINDEXCOUNTER_NAME, 0, oLightIndexCounter);
    cullingDescriptorSet->updateBuffer(TLIGHTINDEXCOUNTER_NAME, 0, tLightIndexCounter);
    cullingDescriptorSet->updateBuffer(OLIGHTINDEXLIST_NAME, 0, oLightIndexList);
    cullingDescriptorSet->updateBuffer(TLIGHTINDEXLIST_NAME, 0, tLightIndexList);
    cullingDescriptorSet->updateTexture(OLIGHTGRID_NAME, 0, oLightGrid->getDefaultView());
    cullingDescriptorSet->updateTexture(TLIGHTGRID_NAME, 0, tLightGrid->getDefaultView());
    cullingDescriptorSet->writeChanges();
    Gfx::OComputeCommand computeCommand = graphics->createComputeCommand("CullingCommand");
    if (getGlobals().useLightCulling) {
        computeCommand->bindPipeline(cullingEnabledPipeline);
    } else {
        computeCommand->bindPipeline(cullingPipeline);
    }
    computeCommand->bindDescriptor({viewParamsSet, dispatchParamsSet, cullingDescriptorSet, lightEnv->getDescriptorSet()});
    computeCommand->dispatch(numThreadGroups.x, numThreadGroups.y, numThreadGroups.z);
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
    graphics->endDebugRegion();
}

void LightCullingPass::endFrame() {}

void LightCullingPass::publishOutputs() {
    setupFrustums();
    uint32_t viewportWidth = viewport->getWidth();
    uint32_t viewportHeight = viewport->getHeight();
    numThreadGroups = glm::ceil(glm::vec4(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1, 0));
    numThreads = numThreadGroups * glm::uvec4(BLOCK_SIZE, BLOCK_SIZE, 1, 0);
    dispatchParamsSet = dispatchParamsLayout->allocateDescriptorSet();
    dispatchParamsSet->updateConstants("numThreadGroups", 0, &numThreadGroups);
    dispatchParamsSet->updateConstants("numThreads", 0, &numThreads);
    dispatchParamsSet->writeChanges();

    cullingDescriptorLayout = graphics->createDescriptorLayout("pCullingParams");

    // DepthTexture
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = DEPTHATTACHMENT_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .access = Gfx::SE_DESCRIPTOR_ACCESS_SAMPLE_BIT,
    });
    // o_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = OLIGHTINDEXCOUNTER_NAME, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_BIT | Gfx::SE_DESCRIPTOR_ACCESS_WRITE_BIT,});
    // t_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TLIGHTINDEXCOUNTER_NAME, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_BIT | Gfx::SE_DESCRIPTOR_ACCESS_WRITE_BIT,});
    // o_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = OLIGHTINDEXLIST_NAME, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_BIT | Gfx::SE_DESCRIPTOR_ACCESS_WRITE_BIT,});
    // t_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TLIGHTINDEXLIST_NAME, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_BIT | Gfx::SE_DESCRIPTOR_ACCESS_WRITE_BIT,});
    // o_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = OLIGHTGRID_NAME, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_BIT | Gfx::SE_DESCRIPTOR_ACCESS_WRITE_BIT,});
    // t_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TLIGHTGRID_NAME, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_BIT | Gfx::SE_DESCRIPTOR_ACCESS_WRITE_BIT,});

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
                .size = (uint32)sizeof(uint32) * numThreadGroups.x * numThreadGroups.y *
                        numThreadGroups.z * 8192,
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
        .width = numThreadGroups.x,
        .height = numThreadGroups.y,
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
    depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH").getTextureView();
}

void LightCullingPass::setupFrustums() {
    graphics->beginDebugRegion("SetupFrustums");
    uint32_t viewportWidth = viewport->getWidth();
    uint32_t viewportHeight = viewport->getHeight();

    numThreads = glm::ceil(glm::vec4(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1, 0));
    numThreadGroups = glm::ceil(glm::vec4(numThreads.x / (float)BLOCK_SIZE, numThreads.y / (float)BLOCK_SIZE, 1, 0));

    updateViewParameters(Component::Camera(), Component::Transform());
    viewParamsSet = createViewParamsSet();

    dispatchParamsLayout = graphics->createDescriptorLayout("pDispatchParams");
    dispatchParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "numThreadGroups",
        .uniformLength = sizeof(UVector4),
    });
    dispatchParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = "numThreads",
        .uniformLength = sizeof(UVector4),
    });
    dispatchParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = FRUSTUMBUFFER_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .access = Gfx::SE_DESCRIPTOR_ACCESS_WRITE_BIT,
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
        .dumpIntermediate = true,
    };
    graphics->beginShaderCompilation(createInfo);
    frustumShader = graphics->createComputeShader({0});
    // Have to compile shader before finalizing layout as parameters get mapped later
    frustumLayout->create();

    Gfx::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = frustumShader;
    pipelineInfo.pipelineLayout = frustumLayout;
    frustumPipeline = graphics->createComputePipeline(pipelineInfo);

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

    dispatchParamsSet = dispatchParamsLayout->allocateDescriptorSet();
    dispatchParamsSet->updateConstants("numThreadGroups", 0, &numThreadGroups);
    dispatchParamsSet->updateConstants("numThreads", 0, &numThreads);
    dispatchParamsSet->updateBuffer(FRUSTUMBUFFER_NAME, 0, frustumBuffer);
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
    graphics->endDebugRegion();
}
