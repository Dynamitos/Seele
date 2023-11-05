#include "LightCullingPass.h"
#include "Graphics/Graphics.h"
#include "Scene/Scene.h"
#include "Actor/CameraActor.h"
#include "Component/Camera.h"
#include "RenderGraph.h"

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
    uint32_t viewportWidth = viewport->getSizeX();
    uint32_t viewportHeight = viewport->getSizeY();

    uint32 reset = 0;
    DataSource counterReset = {
        .size = sizeof(uint32),
        .data = (uint8*)&reset,
    };
    oLightIndexCounter->updateContents(counterReset);
    tLightIndexCounter->updateContents(counterReset);

    cullingDescriptorLayout->reset();
    cullingDescriptorSet = cullingDescriptorLayout->allocateDescriptorSet();

    cullingDescriptorSet->updateBuffer(0, dispatchParamsBuffer);
    cullingDescriptorSet->updateTexture(1, depthAttachment);
    cullingDescriptorSet->updateBuffer(2, oLightIndexCounter);
    cullingDescriptorSet->updateBuffer(3, tLightIndexCounter);
    cullingDescriptorSet->updateBuffer(4, oLightIndexList);
    cullingDescriptorSet->updateBuffer(5, tLightIndexList);
    cullingDescriptorSet->updateTexture(6, Gfx::PTexture2D(oLightGrid));
    cullingDescriptorSet->updateTexture(7, Gfx::PTexture2D(tLightGrid));
    cullingDescriptorSet->updateBuffer(8, frustumBuffer);
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
    cullingDescriptorSet->updateTexture(2, depthAttachment);
    cullingDescriptorSet->writeChanges();
    Gfx::PComputeCommand computeCommand = graphics->createComputeCommand("CullingCommand");
    computeCommand->bindPipeline(cullingPipeline);
    computeCommand->bindDescriptor({ viewParamsSet, lightEnv->getDescriptorSet(), cullingDescriptorSet });
    computeCommand->dispatch(dispatchParams.numThreadGroups.x, dispatchParams.numThreadGroups.y, dispatchParams.numThreadGroups.z);
    Array<Gfx::PComputeCommand> commands = {computeCommand};
    graphics->executeCommands(commands);
    depthAttachment->changeLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    depthAttachment->transferOwnership(Gfx::QueueType::GRAPHICS);
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
    uint32_t viewportWidth = viewport->getSizeX();
    uint32_t viewportHeight = viewport->getSizeY();
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
    
    // Dispatchparams
    cullingDescriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    //DepthTexture
    cullingDescriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    //o_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //t_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //t_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(5, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(6, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    //t_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(7, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    //Frustums
    cullingDescriptorLayout->addDescriptorBinding(8, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, viewportWidth * viewportHeight);

    lightEnv = scene->getLightEnvironment();

    cullingLayout = graphics->createPipelineLayout();
    cullingLayout->addDescriptorLayout(0, viewParamsLayout);
    cullingLayout->addDescriptorLayout(1, lightEnv->getDescriptorLayout());
    cullingLayout->addDescriptorLayout(2, cullingDescriptorLayout);
    cullingLayout->create();
    
    ShaderCreateInfo createInfo;
    createInfo.name = "Culling";
    
    createInfo.mainModule = "LightCulling";
    createInfo.entryPoint = "cullLights";
    cullingShader = graphics->createComputeShader(createInfo);

    Gfx::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = cullingShader;
    pipelineInfo.pipelineLayout = cullingLayout;
    cullingPipeline = graphics->createComputePipeline(pipelineInfo);

    uint32 counterReset = 0;
    ShaderBufferCreateInfo structInfo = 
    {
        .sourceData = {
            .size = sizeof(uint32),
            .data = (uint8*)&counterReset,
            .owner = Gfx::QueueType::COMPUTE,
        },
        .stride = sizeof(uint32),
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
        .stride = sizeof(uint32),
        .dynamic = false,
    };
    oLightIndexList = graphics->createShaderBuffer(structInfo);
    tLightIndexList = graphics->createShaderBuffer(structInfo);
    resources->registerBufferOutput("LIGHTCULLING_OLIGHTLIST", oLightIndexList);
    resources->registerBufferOutput("LIGHTCULLING_TLIGHTLIST", tLightIndexList);
    
    TextureCreateInfo textureInfo = {
        .width = dispatchParams.numThreadGroups.x,
        .height = dispatchParams.numThreadGroups.y,
        .format = Gfx::SE_FORMAT_R32G32_UINT,
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
    uint32_t viewportWidth = viewport->getSizeX();
    uint32_t viewportHeight = viewport->getSizeY();

    glm::uvec3 numThreads = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
    glm::uvec3 numThreadGroups = glm::ceil(glm::vec3(numThreads.x / (float)BLOCK_SIZE, numThreads.y / (float)BLOCK_SIZE, 1));
    
    RenderPass::beginFrame(Component::Camera());
    dispatchParams.numThreads = numThreads;
    dispatchParams.numThreadGroups = numThreadGroups;

    Gfx::ODescriptorLayout frustumDescriptorLayout = graphics->createDescriptorLayout("FrustumLayout");
    frustumDescriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    frustumDescriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    frustumLayout = graphics->createPipelineLayout();
    frustumLayout->addDescriptorLayout(0, viewParamsLayout);
    frustumLayout->addDescriptorLayout(1, frustumDescriptorLayout);
    frustumLayout->create();
    ShaderCreateInfo createInfo;
    createInfo.name = "Frustum";
    
    createInfo.mainModule = "ComputeFrustums";
    createInfo.entryPoint = "computeFrustums";
    frustumShader = graphics->createComputeShader(createInfo);

    Gfx::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = frustumShader;
    pipelineInfo.pipelineLayout = frustumLayout;
    frustumPipeline = graphics->createComputePipeline(pipelineInfo);
    
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
        .stride = sizeof(Frustum),
        .dynamic = false,
    });
    
    frustumDescriptorSet = frustumDescriptorLayout->allocateDescriptorSet();
    frustumDescriptorSet->updateBuffer(0, frustumDispatchParamsBuffer);
    frustumDescriptorSet->updateBuffer(1, frustumBuffer);
    frustumDescriptorSet->writeChanges();
    
    Gfx::PComputeCommand command = graphics->createComputeCommand("FrustumCommand");
    command->bindPipeline(frustumPipeline);
    command->bindDescriptor({ viewParamsSet, frustumDescriptorSet });
    command->dispatch(numThreadGroups.x, numThreadGroups.y, numThreadGroups.z);
    Array<Gfx::PComputeCommand> commands = {command};
    graphics->executeCommands(commands);
    frustumBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}