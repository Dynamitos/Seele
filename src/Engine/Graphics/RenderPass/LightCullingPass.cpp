#include "LightCullingPass.h"
#include "Graphics/Graphics.h"
#include "Scene/Scene.h"
#include "Actor/CameraActor.h"
#include "Component/Camera.h"
#include "RenderGraph.h"

using namespace Seele;

LightCullingPass::LightCullingPass(Gfx::PGraphics graphics)
    : RenderPass(graphics)
{
}

LightCullingPass::~LightCullingPass() 
{
    
}

void LightCullingPass::beginFrame(const Component::Camera& cam) 
{
    uint32_t viewportWidth = viewport->getSizeX();
    uint32_t viewportHeight = viewport->getSizeY();

    BulkResourceData uniformUpdate;
    viewParams.viewMatrix = cam.getViewMatrix();
    viewParams.projectionMatrix = viewport->getProjectionMatrix();
    viewParams.cameraPosition = Math::Vector4(cam.getCameraPosition(), 0);
    viewParams.screenDimensions = Math::Vector2(static_cast<float>(viewportWidth), static_cast<float>(viewportHeight));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamsBuffer->updateContents(uniformUpdate);
    
    const LightEnv& lightEnv = passData.lightEnv;
    uniformUpdate.size = sizeof(DirectionalLight) * MAX_DIRECTIONAL_LIGHTS;
    uniformUpdate.data = (uint8*)&lightEnv.directionalLights;
    directLightBuffer->updateContents(uniformUpdate);
    uniformUpdate.size = sizeof(PointLight) * MAX_POINT_LIGHTS;
    uniformUpdate.data = (uint8*)&lightEnv.pointLights;
    pointLightBuffer->updateContents(uniformUpdate);
    
    uniformUpdate.size = sizeof(uint32);
    uniformUpdate.data = (uint8*)&lightEnv.numDirectionalLights;
    numDirLightBuffer->updateContents(uniformUpdate);
    uniformUpdate.data = (uint8*)&lightEnv.numPointLights;
    numPointLightBuffer->updateContents(uniformUpdate);

    BulkResourceData counterReset;
    uint32 reset = 0;
    counterReset.data = (uint8*)&reset;
    counterReset.size = sizeof(uint32);
    oLightIndexCounter->updateContents(counterReset);
    tLightIndexCounter->updateContents(counterReset);

    cullingDescriptorLayout->reset();
    lightEnvDescriptorLayout->reset();
    cullingDescriptorSet = cullingDescriptorLayout->allocateDescriptorSet();
    lightEnvDescriptorSet = lightEnvDescriptorLayout->allocateDescriptorSet();

    cullingDescriptorSet->updateBuffer(0, viewParamsBuffer);
    cullingDescriptorSet->updateBuffer(1, dispatchParamsBuffer);
    cullingDescriptorSet->updateBuffer(3, oLightIndexCounter);
    cullingDescriptorSet->updateBuffer(4, tLightIndexCounter);
    cullingDescriptorSet->updateBuffer(5, oLightIndexList);
    cullingDescriptorSet->updateBuffer(6, tLightIndexList);
    cullingDescriptorSet->updateTexture(7, oLightGrid);
    cullingDescriptorSet->updateTexture(8, tLightGrid);
    cullingDescriptorSet->updateBuffer(9, frustumBuffer);


    lightEnvDescriptorSet->updateBuffer(0, directLightBuffer);
    lightEnvDescriptorSet->updateBuffer(1, numDirLightBuffer);
    lightEnvDescriptorSet->updateBuffer(2, pointLightBuffer);
    lightEnvDescriptorSet->updateBuffer(3, numPointLightBuffer);
    lightEnvDescriptorSet->writeChanges();
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
    Array<Gfx::PDescriptorSet> descriptorSets = {cullingDescriptorSet, lightEnvDescriptorSet};
    computeCommand->bindDescriptor(descriptorSets);
    //computeCommand->dispatch(dispatchParams.numThreadGroups.x, dispatchParams.numThreadGroups.y, dispatchParams.numThreadGroups.z);
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

    cullingDescriptorLayout = graphics->createDescriptorLayout("CullingLayout");
    
    //ViewParams
    cullingDescriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    //Dispatchparams
    cullingDescriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    //DepthTexture
    cullingDescriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    //o_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //t_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(5, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //t_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(6, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(7, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    //t_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(8, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    //Frustums
    cullingDescriptorLayout->addDescriptorBinding(9, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, viewportWidth * viewportHeight);


    lightEnvDescriptorLayout = graphics->createDescriptorLayout("LightEnv");
    // Directional Lights
    lightEnvDescriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_DIRECTIONAL_LIGHTS);
    lightEnvDescriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    // Point Lights
    lightEnvDescriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_POINT_LIGHTS);
    lightEnvDescriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    cullingLayout = graphics->createPipelineLayout();
    cullingLayout->addDescriptorLayout(0, cullingDescriptorLayout);
    cullingLayout->addDescriptorLayout(1, lightEnvDescriptorLayout);
    cullingLayout->create();
    
    ShaderCreateInfo createInfo;
    createInfo.name = "Culling";
    
    createInfo.mainModule = "LightCulling";
    createInfo.entryPoint = "cullLights";
    createInfo.defines["INDEX_VIEW_PARAMS"] = "0";
    createInfo.defines["INDEX_LIGHT_ENV"] = "1";
    createInfo.defines["NUM_MATERIAL_TEXCOORDS"] = "0";
    cullingShader = graphics->createComputeShader(createInfo);

    ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = cullingShader;
    pipelineInfo.pipelineLayout = cullingLayout;
    cullingPipeline = graphics->createComputePipeline(pipelineInfo);

    uint32 counterReset = 0;
    StructuredBufferCreateInfo structInfo = 
    {
        .resourceData = {
            .size = sizeof(uint32),
            .data = (uint8*)&counterReset,
            .owner = Gfx::QueueType::COMPUTE,
        },
        .stride = sizeof(uint32),
        .bDynamic = true,
    };
    oLightIndexCounter = graphics->createStructuredBuffer(structInfo);
    tLightIndexCounter = graphics->createStructuredBuffer(structInfo);
    structInfo = {
        .resourceData = {
            .size = (uint32)sizeof(uint32) 
                * dispatchParams.numThreadGroups.x 
                * dispatchParams.numThreadGroups.y 
                * dispatchParams.numThreadGroups.z * 200,
            .data = nullptr,
            .owner = Gfx::QueueType::COMPUTE
        },
        .stride = sizeof(uint32),
        .bDynamic = false,
    };
    oLightIndexList = graphics->createStructuredBuffer(structInfo);
    tLightIndexList = graphics->createStructuredBuffer(structInfo);
    resources->registerBufferOutput("LIGHTCULLING_OLIGHTLIST", oLightIndexList);
    resources->registerBufferOutput("LIGHTCULLING_TLIGHTLIST", tLightIndexList);
    
    structInfo = {
        .resourceData = {
            .size = sizeof(DirectionalLight) * MAX_DIRECTIONAL_LIGHTS,
            .data = nullptr,
        },
        .stride = sizeof(DirectionalLight),
        .bDynamic = true,
    };
    directLightBuffer = graphics->createStructuredBuffer(structInfo);
    structInfo.resourceData.size = sizeof(PointLight) * MAX_POINT_LIGHTS;
    pointLightBuffer = graphics->createStructuredBuffer(structInfo);
    
    UniformBufferCreateInfo uniformInfo = {
        .resourceData = {
            .size = sizeof(uint32),
            .data = nullptr
        },
        .bDynamic = true
    };
    numDirLightBuffer = graphics->createUniformBuffer(uniformInfo);
    numPointLightBuffer = graphics->createUniformBuffer(uniformInfo);
    
    resources->registerBufferOutput("DIRECTIONAL_LIGHTS", directLightBuffer);
    resources->registerUniformOutput("NUM_DIRECTIONAL_LIGHTS", numDirLightBuffer);
    resources->registerBufferOutput("POINT_LIGHTS", pointLightBuffer);
    resources->registerUniformOutput("NUM_POINT_LIGHTS", numPointLightBuffer);
    
    TextureCreateInfo textureInfo = {
        .width = dispatchParams.numThreadGroups.x,
        .height = dispatchParams.numThreadGroups.y,
        .format = Gfx::SE_FORMAT_R32G32_UINT,
        .usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT,
    };
    oLightGrid = graphics->createTexture2D(textureInfo);
    tLightGrid = graphics->createTexture2D(textureInfo);
    
    resources->registerTextureOutput("LIGHTCULLING_OLIGHTGRID", oLightGrid);
    resources->registerTextureOutput("LIGHTCULLING_TLIGHTGRID", tLightGrid);
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
    
    viewParams = {
        .viewMatrix = Math::Matrix4(),
        .projectionMatrix = Math::Matrix4(),
        .cameraPosition = Math::Vector4(),
        .screenDimensions = glm::vec2(viewportWidth, viewportHeight),
    };
    dispatchParams.numThreads = numThreads;
    dispatchParams.numThreadGroups = numThreadGroups;

    Gfx::PDescriptorLayout frustumDescriptorLayout = graphics->createDescriptorLayout("FrustumLayout");
    frustumDescriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    frustumDescriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    frustumDescriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    frustumLayout = graphics->createPipelineLayout();
    frustumLayout->addDescriptorLayout(0, frustumDescriptorLayout);
    frustumLayout->create();
    ShaderCreateInfo createInfo;
    createInfo.name = "Frustum";
    
    createInfo.mainModule = "ComputeFrustums";
    createInfo.entryPoint = "computeFrustums";
    createInfo.defines["INDEX_VIEW_PARAMS"] = "0";
    frustumShader = graphics->createComputeShader(createInfo);

    ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = frustumShader;
    pipelineInfo.pipelineLayout = frustumLayout;
    frustumPipeline = graphics->createComputePipeline(pipelineInfo);
    
    BulkResourceData resourceInfo;
    UniformBufferCreateInfo uniformInfo;
    resourceInfo.size = sizeof(ViewParameter);
    resourceInfo.data = (uint8*)&viewParams;
    resourceInfo.owner = Gfx::QueueType::COMPUTE;
    uniformInfo.resourceData = resourceInfo;
    uniformInfo.bDynamic = false;
    viewParamsBuffer = graphics->createUniformBuffer(uniformInfo);

    resourceInfo.size = sizeof(DispatchParams);
    resourceInfo.data = (uint8*)&dispatchParams;
    uniformInfo.resourceData = resourceInfo;
    uniformInfo.bDynamic = false;
    dispatchParamsBuffer = graphics->createUniformBuffer(uniformInfo);

    StructuredBufferCreateInfo structuredInfo = {
        .resourceData = {
            .size = sizeof(Frustum) * numThreads.x * numThreads.y * numThreads.z,
            .data = nullptr,
        },
        .stride = sizeof(Frustum),
        .bDynamic = false,
    };
    frustumBuffer = graphics->createStructuredBuffer(structuredInfo);
    
    frustumDescriptorSet = frustumDescriptorLayout->allocateDescriptorSet();
    frustumDescriptorSet->updateBuffer(0, viewParamsBuffer);
    frustumDescriptorSet->updateBuffer(1, dispatchParamsBuffer);
    frustumDescriptorSet->updateBuffer(2, frustumBuffer);
    frustumDescriptorSet->writeChanges();
    
    Gfx::PComputeCommand command = graphics->createComputeCommand("FrustumCommand");
    command->bindPipeline(frustumPipeline);
    command->bindDescriptor(frustumDescriptorSet);
    command->dispatch(numThreadGroups.x, numThreadGroups.y, numThreadGroups.z);
    Array<Gfx::PComputeCommand> commands = {command};
    graphics->executeCommands(commands);
    frustumBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}