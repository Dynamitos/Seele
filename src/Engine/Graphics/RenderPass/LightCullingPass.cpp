#include "LightCullingPass.h"
#include "Graphics/Graphics.h"
#include "Scene/Scene.h"
#include "Scene/Actor/CameraActor.h"
#include "Scene/Components/CameraComponent.h"
#include "RenderGraph.h"

using namespace Seele;

LightCullingPass::LightCullingPass(Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor camera)
    : RenderPass(graphics, viewport)
    , source(camera->getCameraComponent())
{
}

LightCullingPass::~LightCullingPass() 
{
    
}

void LightCullingPass::beginFrame() 
{
    uint32_t viewportWidth = viewport->getSizeX();
    uint32_t viewportHeight = viewport->getSizeY();

    BulkResourceData uniformUpdate;
    viewParams.viewMatrix = source->getViewMatrix();
    viewParams.projectionMatrix = source->getProjectionMatrix();
    viewParams.cameraPosition = Vector4(source->getCameraPosition(), 0);
    viewParams.inverseProjectionMatrix = glm::inverse(viewParams.projectionMatrix);
    viewParams.screenDimensions = Vector2(static_cast<float>(viewportWidth), static_cast<float>(viewportHeight));
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
    cullingDescriptorSet->updateBuffer(3, frustumBuffer);
    cullingDescriptorSet->updateBuffer(4, oLightIndexCounter);
    cullingDescriptorSet->updateBuffer(5, tLightIndexCounter);
    cullingDescriptorSet->updateBuffer(6, oLightIndexList);
    cullingDescriptorSet->updateBuffer(7, tLightIndexList);
    cullingDescriptorSet->updateTexture(8, oLightGrid);
    cullingDescriptorSet->updateTexture(9, tLightGrid);


    lightEnvDescriptorSet->updateBuffer(0, directLightBuffer);
    lightEnvDescriptorSet->updateBuffer(1, numDirLightBuffer);
    lightEnvDescriptorSet->updateBuffer(2, pointLightBuffer);
    lightEnvDescriptorSet->updateBuffer(3, numPointLightBuffer);
    lightEnvDescriptorSet->writeChanges();
}

MainJob LightCullingPass::render() 
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
    computeCommand->dispatch(dispatchParams.numThreadGroups.x, dispatchParams.numThreadGroups.y, dispatchParams.numThreadGroups.z);
    Array<Gfx::PComputeCommand> commands = {computeCommand};
    graphics->executeCommands(commands);
    depthAttachment->changeLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    depthAttachment->transferOwnership(Gfx::QueueType::GRAPHICS);
    co_return;
}

void LightCullingPass::endFrame() 
{
}

void LightCullingPass::publishOutputs() 
{
    setupFrustums();
    uint32_t viewportWidth = viewport->getSizeX();
    uint32_t viewportHeight = viewport->getSizeY();
    glm::uvec3 numThreadGroups = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
    dispatchParams.numThreadGroups = numThreadGroups;
    dispatchParams.numThreads = numThreadGroups * glm::uvec3(BLOCK_SIZE, BLOCK_SIZE, 1);

    BulkResourceData resourceData;
    StructuredBufferCreateInfo structInfo;
    uint32 counterReset = 0;
    resourceData.size = sizeof(uint32);
    resourceData.data = (uint8*)&counterReset;
    resourceData.owner = Gfx::QueueType::COMPUTE;
    structInfo.bDynamic = true;
    structInfo.resourceData = resourceData;
    oLightIndexCounter = graphics->createStructuredBuffer(structInfo);
    tLightIndexCounter = graphics->createStructuredBuffer(structInfo);
    resourceData.data = nullptr;
    resourceData.size = sizeof(uint32_t) 
        * dispatchParams.numThreadGroups.x 
        * dispatchParams.numThreadGroups.y 
        * dispatchParams.numThreadGroups.z * 200;
    structInfo.resourceData = resourceData;
    structInfo.bDynamic = false;
    oLightIndexList = graphics->createStructuredBuffer(structInfo);
    tLightIndexList = graphics->createStructuredBuffer(structInfo);
    resources->registerBufferOutput("LIGHTCULLING_OLIGHTLIST", oLightIndexList);
    resources->registerBufferOutput("LIGHTCULLING_TLIGHTLIST", tLightIndexList);
    
    resourceData.size = sizeof(DirectionalLight) * MAX_DIRECTIONAL_LIGHTS;
    structInfo.resourceData = resourceData;
    structInfo.bDynamic = true;
    directLightBuffer = graphics->createStructuredBuffer(structInfo);
    resourceData.size = sizeof(PointLight) * MAX_POINT_LIGHTS;
    structInfo.resourceData = resourceData;
    pointLightBuffer = graphics->createStructuredBuffer(structInfo);
    
    UniformBufferCreateInfo uniformInfo;
    resourceData.size = sizeof(uint32);
    uniformInfo.resourceData = resourceData;
    uniformInfo.bDynamic = true;
    numDirLightBuffer = graphics->createUniformBuffer(uniformInfo);
    uniformInfo.resourceData = resourceData;
    uniformInfo.bDynamic = true;
    numPointLightBuffer = graphics->createUniformBuffer(uniformInfo);
    
    resources->registerBufferOutput("DIRECTIONAL_LIGHTS", directLightBuffer);
    resources->registerUniformOutput("NUM_DIRECTIONAL_LIGHTS", numDirLightBuffer);
    resources->registerBufferOutput("POINT_LIGHTS", pointLightBuffer);
    resources->registerUniformOutput("NUM_POINT_LIGHTS", numPointLightBuffer);
    
    TextureCreateInfo textureInfo;
    textureInfo.width = dispatchParams.numThreadGroups.x;
    textureInfo.height = dispatchParams.numThreadGroups.y;
    textureInfo.format = Gfx::SE_FORMAT_R32G32_UINT;
    textureInfo.usage = Gfx::SE_IMAGE_USAGE_STORAGE_BIT;
    oLightGrid = graphics->createTexture2D(textureInfo);
    tLightGrid = graphics->createTexture2D(textureInfo);
    
    resources->registerTextureOutput("LIGHTCULLING_OLIGHTGRID", oLightGrid);
    resources->registerTextureOutput("LIGHTCULLING_TLIGHTGRID", tLightGrid);
}


void LightCullingPass::createRenderPass()
{
    cullingDescriptorLayout = graphics->createDescriptorLayout("CullingLayout");
    
    //ViewParams
    cullingDescriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    //Dispatchparams
    cullingDescriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    //DepthTexture
    cullingDescriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    //Frustums
    cullingDescriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //t_lightIndexCounter
    cullingDescriptorLayout->addDescriptorBinding(5, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(6, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //t_lightIndexList
    cullingDescriptorLayout->addDescriptorBinding(7, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    //o_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(8, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    //t_lightGrid
    cullingDescriptorLayout->addDescriptorBinding(9, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);


    lightEnvDescriptorLayout = graphics->createDescriptorLayout("LightEnv");
    // Directional Lights
    lightEnvDescriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    lightEnvDescriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    // Point Lights
    lightEnvDescriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    lightEnvDescriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    cullingLayout = graphics->createPipelineLayout();
    cullingLayout->addDescriptorLayout(0, cullingDescriptorLayout);
    cullingLayout->addDescriptorLayout(1, lightEnvDescriptorLayout);
    cullingLayout->create();
    
    ShaderCreateInfo createInfo;
    createInfo.name = "Culling";
    
    std::ifstream codeStream("./shaders/LightCulling.slang", std::ios::ate);
    auto fileSize = codeStream.tellg();
    codeStream.seekg(0);
    Array<char> buffer(static_cast<uint32>(fileSize));
    codeStream.read(buffer.data(), fileSize);

    createInfo.shaderCode.add(std::string(buffer.data()));
    createInfo.entryPoint = "cullLights";
    createInfo.defines["INDEX_VIEW_PARAMS"] = "0";
    createInfo.defines["INDEX_LIGHT_ENV"] = "1";
    createInfo.defines["NUM_MATERIAL_TEXCOORDS"] = "0";
    cullingShader = graphics->createComputeShader(createInfo);

    ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = cullingShader;
    pipelineInfo.pipelineLayout = cullingLayout;
    cullingPipeline = graphics->createComputePipeline(pipelineInfo);
    
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
    
    viewParams.viewMatrix = source->getViewMatrix();
    viewParams.projectionMatrix = source->getProjectionMatrix();
    viewParams.inverseProjectionMatrix = glm::inverse(source->getProjectionMatrix());
    viewParams.screenDimensions = glm::vec2(viewportWidth, viewportHeight);
    viewParams.cameraPosition = Vector4(source->getCameraPosition(), 0);
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
    
    std::ifstream codeStream("./shaders/ComputeFrustums.slang", std::ios::ate);
    auto fileSize = codeStream.tellg();
    codeStream.seekg(0);
    Array<char> buffer(static_cast<uint32>(fileSize));
    codeStream.read(buffer.data(), fileSize);

    createInfo.shaderCode.add(std::string(buffer.data()));
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

    StructuredBufferCreateInfo structuredInfo;
    resourceInfo.size = sizeof(Frustum) * numThreads.x * numThreads.y * numThreads.z;
    resourceInfo.data = nullptr;
    structuredInfo.resourceData = resourceInfo;
    structuredInfo.bDynamic = false;
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