#include "LightCullingPass.h"
#include "Graphics/Graphics.h"
#include "Scene/Scene.h"
#include "Actor/CameraActor.h"
#include "Component/Camera.h"
#include "RenderGraph.h"
#include "Graphics/Command.h"

using namespace Seele;

LightCullingPass::LightCullingPass(Gfx::PGraphics graphics, PScene scene)
    : RenderPass(graphics, scene)
{
}

LightCullingPass::~LightCullingPass() 
{
    
}

void LightCullingPass::beginFrame(const Component::Camera& cam) 
{
    RenderPass::beginFrame(cam);

    uint32 reset = 0;
    DataSource counterReset = {
        .size = sizeof(uint32),
        .data = (uint8*)&reset,
    };
    oLightIndexCounter->updateContents(counterReset);
    tLightIndexCounter->updateContents(counterReset);

    cullingDescriptorLayout->reset();
    cullingDescriptorSet = cullingDescriptorLayout->allocateDescriptorSet();

    cullingDescriptorSet->updateBuffer(1, oLightIndexCounter);
    cullingDescriptorSet->updateBuffer(2, tLightIndexCounter);
    cullingDescriptorSet->updateBuffer(3, oLightIndexList);
    cullingDescriptorSet->updateBuffer(4, tLightIndexList);
    cullingDescriptorSet->updateTexture(5, Gfx::PTexture2D(oLightGrid));
    cullingDescriptorSet->updateTexture(6, Gfx::PTexture2D(tLightGrid));
    //std::cout << "LightCulling beginFrame()" << std::endl;
    //co_return;
}

void LightCullingPass::render() 
{
    oLightIndexList->pipelineBarrier( 
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    oLightGrid->pipelineBarrier( 
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    depthAttachment->pipelineBarrier(
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    depthAttachment->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL);
    depthAttachment->transferOwnership(Gfx::QueueType::COMPUTE);
    cullingDescriptorSet->updateTexture(0, depthAttachment);
    cullingDescriptorSet->writeChanges();
    Gfx::PComputeCommand computeCommand = graphics->createComputeCommand("CullingCommand");
    computeCommand->bindPipeline(cullingPipeline);
    computeCommand->bindDescriptor({ viewParamsSet, dispatchParamsSet, cullingDescriptorSet, lightEnv->getDescriptorSet() });
    //computeCommand->dispatch(dispatchParams.numThreadGroups.x, dispatchParams.numThreadGroups.y, dispatchParams.numThreadGroups.z);
    Array<Gfx::PComputeCommand> commands = {computeCommand};
    graphics->executeCommands(commands);
    //std::cout << "LightCulling render()" << std::endl;
    //co_return;
}

void LightCullingPass::endFrame() 
{
    //std::cout << "LightCulling endFrame()" << std::endl;
    //co_return;
}

void LightCullingPass::publishOutputs() 
{
    setupFrustums();
    uint32_t viewportWidth = viewport->getWidth();
    uint32_t viewportHeight = viewport->getHeight();
    glm::uvec3 numThreadGroups = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
    dispatchParams.numThreadGroups = numThreadGroups;
    dispatchParams.numThreads = numThreadGroups * glm::uvec3(BLOCK_SIZE, BLOCK_SIZE, 1);
    dispatchParamsBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData = {
            .size = sizeof(DispatchParams),
            .data = (uint8*)&dispatchParams,
        },
        .dynamic = false,
        });

    cullingDescriptorLayout = graphics->createDescriptorLayout("CullingLayout");

    //DepthTexture
    cullingDescriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    //o_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //t_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //t_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(5, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    //t_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(6, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    lightEnv = scene->getLightEnvironment();

    Gfx::OPipelineLayout cullingLayout = graphics->createPipelineLayout();
    cullingLayout->addDescriptorLayout(0, viewParamsLayout);
    cullingLayout->addDescriptorLayout(1, dispatchParamsLayout);
    cullingLayout->addDescriptorLayout(2, cullingDescriptorLayout);
    cullingLayout->addDescriptorLayout(3, lightEnv->getDescriptorLayout());
    cullingLayout->create();
    
    ShaderCreateInfo createInfo;
    createInfo.name = "Culling";
    createInfo.additionalModules.add("LightCulling");
    createInfo.mainModule = "LightCulling";
    createInfo.entryPoint = "cullLights";
    cullingShader = graphics->createComputeShader(createInfo);

    Gfx::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = cullingShader;
    pipelineInfo.pipelineLayout = std::move(cullingLayout);
    cullingPipeline = graphics->createComputePipeline(std::move(pipelineInfo));

    uint32 counterReset = 0;
    ShaderBufferCreateInfo structInfo = {
        .sourceData = {
            .size = sizeof(uint32),
            .data = (uint8*)&counterReset,
            .owner = Gfx::QueueType::COMPUTE,
        },
        .numElements = 1,
        .dynamic = true,
    };
    oLightIndexCounter = graphics->createShaderBuffer(structInfo);
    tLightIndexCounter = graphics->createShaderBuffer(structInfo);
    structInfo = {
        .sourceData = {
            .size = (uint32)sizeof(uint32) 
                * dispatchParams.numThreadGroups.x 
                * dispatchParams.numThreadGroups.y 
                * dispatchParams.numThreadGroups.z * 200,
            .data = nullptr,
            .owner = Gfx::QueueType::COMPUTE
        },
        .dynamic = false,
    };
    oLightIndexList = graphics->createShaderBuffer(structInfo);
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
    
    resources->registerTextureOutput("LIGHTCULLING_OLIGHTGRID", Gfx::PTexture2D(oLightGrid));
    resources->registerTextureOutput("LIGHTCULLING_TLIGHTGRID", Gfx::PTexture2D(tLightGrid));
}


void LightCullingPass::createRenderPass()
{
    depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH")->getTexture();
}

void LightCullingPass::modifyRenderPassMacros(Map<const char*, const char*>&) 
{
}

void LightCullingPass::setupFrustums()
{
    uint32_t viewportWidth = viewport->getWidth();
    uint32_t viewportHeight = viewport->getHeight();

    glm::uvec3 numThreads = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
    glm::uvec3 numThreadGroups = glm::ceil(glm::vec3(numThreads.x / (float)BLOCK_SIZE, numThreads.y / (float)BLOCK_SIZE, 1));
    
    RenderPass::beginFrame(Component::Camera());
    dispatchParams.numThreads = numThreads;
    dispatchParams.numThreadGroups = numThreadGroups;

    dispatchParamsLayout = graphics->createDescriptorLayout("FrustumLayout");
    dispatchParamsLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    dispatchParamsLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    Gfx::OPipelineLayout frustumLayout = graphics->createPipelineLayout();
    frustumLayout->addDescriptorLayout(0, viewParamsLayout);
    frustumLayout->addDescriptorLayout(1, dispatchParamsLayout);
    frustumLayout->create();
    ShaderCreateInfo createInfo;
    createInfo.name = "Frustum";
    createInfo.additionalModules.add("ComputeFrustums");
    createInfo.mainModule = "ComputeFrustums";
    createInfo.entryPoint = "computeFrustums";
    frustumShader = graphics->createComputeShader(createInfo);

    Gfx::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = frustumShader;
    pipelineInfo.pipelineLayout = std::move(frustumLayout);
    frustumPipeline = graphics->createComputePipeline(std::move(pipelineInfo));
    
    Gfx::OUniformBuffer frustumDispatchParamsBuffer = graphics->createUniformBuffer(UniformBufferCreateInfo{
        .sourceData = {
            .size = sizeof(DispatchParams),
            .data = (uint8*) & dispatchParams,
        },
        .dynamic = false,
    });
    
    frustumBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData = {
            .size = sizeof(Frustum) * numThreads.x * numThreads.y * numThreads.z,
            .data = nullptr,
        },
        .numElements = numThreads.x * numThreads.y * numThreads.z,
        .dynamic = false,
    });
    
    dispatchParamsSet = dispatchParamsLayout->allocateDescriptorSet();
    dispatchParamsSet->updateBuffer(0, frustumDispatchParamsBuffer);
    dispatchParamsSet->updateBuffer(1, frustumBuffer);
    dispatchParamsSet->writeChanges();
    
    Gfx::PComputeCommand command = graphics->createComputeCommand("FrustumCommand");
    command->bindPipeline(frustumPipeline);
    command->bindDescriptor({ viewParamsSet, dispatchParamsSet });
    command->dispatch(numThreadGroups.x, numThreadGroups.y, numThreadGroups.z);
    Array<Gfx::PComputeCommand> commands = {command};
    graphics->executeCommands(commands);
    frustumBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}