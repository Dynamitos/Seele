#include "BasePass.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Material/MaterialInstance.h"
#include "Graphics/Descriptor.h"

using namespace Seele;

BasePass::BasePass(Gfx::PGraphics graphics, PScene scene) 
    : RenderPass(graphics, scene)
    , descriptorSets(4)
{
    UniformBufferCreateInfo uniformInitializer;
    basePassLayout = graphics->createPipelineLayout();

    lightLayout = graphics->createDescriptorLayout("LightLayout");
    
    // Directional Lights
    lightLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    lightLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    // Point Lights
	lightLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    lightLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    // Light Index List
    lightLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    // Light Grid
    lightLayout->addDescriptorBinding(5, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    lightLayout->create();
    basePassLayout->addDescriptorLayout(INDEX_LIGHT_ENV, lightLayout);
    descriptorSets[INDEX_LIGHT_ENV] = lightLayout->allocateDescriptorSet();

    viewLayout = graphics->createDescriptorLayout("ViewLayout");
    viewLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    uniformInitializer.resourceData.size = sizeof(ViewParameter);
    uniformInitializer.resourceData.data = (uint8*)&viewParams;
    uniformInitializer.bDynamic = true;
    viewParamBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewLayout->create();
    basePassLayout->addDescriptorLayout(INDEX_VIEW_PARAMS, viewLayout);
    descriptorSets[INDEX_VIEW_PARAMS] = viewLayout->allocateDescriptorSet();

    sceneLayout = graphics->createDescriptorLayout("SceneLayout");
    sceneLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    sceneLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    sceneLayout->create();
    basePassLayout->addDescriptorLayout(INDEX_SCENE_DATA, sceneLayout);
    //basePassLayout->addPushConstants(Gfx::SePushConstantRange{
    //    .stageFlags = (Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT),
    //    .offset = 0,
    //    .size = sizeof(uint32),
    //});
}

BasePass::~BasePass()
{
}

void BasePass::beginFrame(const Component::Camera& cam) 
{
    BulkResourceData uniformUpdate;

    viewParams.viewMatrix = cam.getViewMatrix();
    viewParams.projectionMatrix = viewport->getProjectionMatrix();
    viewParams.cameraPosition = Vector4(cam.getCameraPosition(), 1);
    viewParams.screenDimensions = Vector2(static_cast<float>(viewport->getSizeX()), static_cast<float>(viewport->getSizeY()));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamBuffer->updateContents(uniformUpdate);
    viewLayout->reset();
    lightLayout->reset();

    descriptorSets[INDEX_SCENE_DATA] = sceneLayout->allocateDescriptorSet();
    descriptorSets[INDEX_SCENE_DATA]->updateBuffer(0, passData.sceneDataBuffer);
    descriptorSets[INDEX_SCENE_DATA]->writeChanges();
    descriptorSets[INDEX_LIGHT_ENV] = lightLayout->allocateDescriptorSet();
    descriptorSets[INDEX_VIEW_PARAMS] = viewLayout->allocateDescriptorSet();
    descriptorSets[INDEX_VIEW_PARAMS]->updateBuffer(0, viewParamBuffer);
    descriptorSets[INDEX_VIEW_PARAMS]->writeChanges();
    //std::cout << "BasePass beginFrame()" << std::endl;
    //co_return;
}

void BasePass::render() 
{
    oLightIndexList->pipelineBarrier( 
		Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	oLightGrid->pipelineBarrier( 
		Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
		Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(0, passData.lightEnv.directionalLights);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(1, passData.lightEnv.numDirectional);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(2, passData.lightEnv.pointLights);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(3, passData.lightEnv.numPoints);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(4, oLightIndexList);
    descriptorSets[INDEX_LIGHT_ENV]->updateTexture(5, oLightGrid);
    descriptorSets[INDEX_LIGHT_ENV]->writeChanges();

    graphics->beginRenderPass(renderPass);
    for (const auto& meshBatch : passData.staticDrawList)
    {
        processor->processMeshBatch(meshBatch, viewport, renderPass, basePassLayout, primitiveLayout, descriptorSets);
    }
    graphics->executeCommands(processor->getRenderCommands());
    graphics->endRenderPass();
    //std::cout << "BasePass render()" << std::endl;
    //co_return;
}

void BasePass::endFrame() 
{
    //std::cout << "BasePass endFrame()" << std::endl;
    //co_return;
}

void BasePass::publishOutputs() 
{
	colorAttachment = new Gfx::SwapchainAttachment(viewport->getOwner());
    resources->registerRenderPassOutput("BASEPASS_COLOR", colorAttachment);
}

void BasePass::createRenderPass() 
{
    Gfx::PRenderTargetAttachment depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment->storeOp = Gfx::SE_ATTACHMENT_STORE_OP_STORE;
    Gfx::PRenderTargetLayout layout = new Gfx::RenderTargetLayout(colorAttachment, depthAttachment);
    renderPass = graphics->createRenderPass(layout, viewport);
    oLightIndexList = resources->requestBuffer("LIGHTCULLING_OLIGHTLIST");
    oLightGrid = resources->requestTexture("LIGHTCULLING_OLIGHTGRID");
}

void BasePass::modifyRenderPassMacros(Map<const char*, const char*>& defines)
{
    defines["INDEX_LIGHT_ENV"] = "0";
    defines["INDEX_VIEW_PARAMS"] = "1";
    defines["INDEX_MATERIAL"] = "2";
    defines["INDEX_SCENE_DATA"] = "3";
}
