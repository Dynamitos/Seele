#include "BasePass.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Component/Camera.h"
#include "Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Material/MaterialAsset.h"

using namespace Seele;

BasePassMeshProcessor::BasePassMeshProcessor(Gfx::PGraphics graphics, uint8 translucentBasePass) 
    : MeshProcessor(graphics)
    , translucentBasePass(translucentBasePass)
    //, cachedPrimitiveIndex(0)
{
}

BasePassMeshProcessor::~BasePassMeshProcessor() 
{ 
}

void BasePassMeshProcessor::processMeshBatch(
    const MeshBatch& batch, 
    Gfx::PViewport target,
    Gfx::PRenderPass renderPass,
    Gfx::PPipelineLayout baseLayout,
    Gfx::PDescriptorLayout primitiveLayout,
    Array<Gfx::PDescriptorSet> descriptorSets,
    int32 /*staticMeshId*/) 
{
    PMaterialAsset material = batch.material;
    //const Gfx::MaterialShadingModel shadingModel = material->getShadingModel();

    const PVertexShaderInput vertexInput = batch.vertexInput;

    const Gfx::ShaderCollection* collection = material->getShaders(Gfx::RenderPassType::BasePass, vertexInput->getType());
    assert(collection != nullptr);

    Gfx::PRenderCommand renderCommand = graphics->createRenderCommand();    
    renderCommand->setViewport(target);
    
    Gfx::PPipelineLayout pipelineLayout = graphics->createPipelineLayout(baseLayout);
    pipelineLayout->addDescriptorLayout(BasePass::INDEX_MATERIAL, material->getDescriptorLayout());
    pipelineLayout->create();
    Gfx::PDescriptorSet materialSet = material->createDescriptorSet();
    descriptorSets[BasePass::INDEX_MATERIAL] = materialSet;
    for(uint32 i = 0; i < batch.elements.size(); ++i)
    {
        Gfx::PDescriptorSet descriptorSet = primitiveLayout->allocateDescriptorSet();
        descriptorSet->updateBuffer(0, batch.elements[i].uniformBuffer);
        descriptorSet->writeChanges();
        descriptorSets[BasePass::INDEX_SCENE_DATA] = descriptorSet;
        buildMeshDrawCommand(batch, 
//            primitiveComponent, 
            renderPass,
            pipelineLayout,
            renderCommand,
            descriptorSets,
            collection->vertexShader, 
            collection->controlShader,
            collection->evalutionShader,
            collection->geometryShader,
            collection->fragmentShader,
            false);
    }
    std::scoped_lock lock(commandLock);
    renderCommands.add(renderCommand);
    //co_return;
}

BasePass::BasePass(Gfx::PGraphics graphics) 
    : RenderPass(graphics)
    , processor(new BasePassMeshProcessor(graphics, false))
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

    primitiveLayout = graphics->createDescriptorLayout("PrimitiveLayout");
    primitiveLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    primitiveLayout->create();
    basePassLayout->addDescriptorLayout(INDEX_SCENE_DATA, primitiveLayout);
}

BasePass::~BasePass()
{   
}

void BasePass::beginFrame(const Component::Camera& cam) 
{
    processor->clearCommands();
    primitiveLayout->reset();
    BulkResourceData uniformUpdate;

    viewParams.viewMatrix = cam.getViewMatrix();
    viewParams.projectionMatrix = cam.getProjectionMatrix();
    viewParams.cameraPosition = Math::Vector4(cam.getCameraPosition(), 0);
    viewParams.screenDimensions = Math::Vector2(static_cast<float>(viewport->getSizeX()), static_cast<float>(viewport->getSizeY()));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamBuffer->updateContents(uniformUpdate);
    viewLayout->reset();
    lightLayout->reset();
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

    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(0, directLightBuffer);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(1, numDirLightBuffer);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(2, pointLightBuffer);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(3, numPointLightBuffer);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(4, oLightIndexList);
    descriptorSets[INDEX_LIGHT_ENV]->updateTexture(5, oLightGrid);
    descriptorSets[INDEX_LIGHT_ENV]->writeChanges();
    graphics->beginRenderPass(renderPass);
    for (auto &&meshBatch : passData.staticDrawList)
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
	directLightBuffer = resources->requestBuffer("DIRECTIONAL_LIGHTS");
	pointLightBuffer = resources->requestBuffer("POINT_LIGHTS");
	numDirLightBuffer = resources->requestUniform("NUM_DIRECTIONAL_LIGHTS");
	numPointLightBuffer = resources->requestUniform("NUM_POINT_LIGHTS");
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
