#include "BasePass.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"

using namespace Seele;

BasePassMeshProcessor::BasePassMeshProcessor(const PScene scene, Gfx::PViewport viewport, Gfx::PGraphics graphics, uint8 translucentBasePass) 
    : MeshProcessor(scene, graphics)
    , target(viewport)
    , translucentBasePass(translucentBasePass)
    , cachedPrimitiveIndex(0)
{
}

BasePassMeshProcessor::~BasePassMeshProcessor() 
{ 
}

void BasePassMeshProcessor::addMeshBatch(
    const MeshBatch& batch, 
//    const PPrimitiveComponent primitiveComponent,
    const Gfx::PRenderPass renderPass,
    Gfx::PPipelineLayout pipelineLayout,
    Gfx::PDescriptorLayout primitiveLayout,
    Array<Gfx::PDescriptorSet>& descriptorSets,
    int32 /*staticMeshId*/) 
{
    const PMaterialAsset material = batch.material;
    //const Gfx::MaterialShadingModel shadingModel = material->getShadingModel();

    const PVertexShaderInput vertexInput = batch.vertexInput;

	const Gfx::ShaderCollection* collection = material->getRenderMaterial()->getShaders(Gfx::RenderPassType::BasePass, vertexInput->getType());
    assert(collection != nullptr);
    for(uint32 i = 0; i < batch.elements.size(); ++i)
    {
        Gfx::PDescriptorSet descriptorSet = primitiveLayout->allocateDescriptorSet();
        descriptorSet->updateBuffer(0, batch.elements[i].uniformBuffer);
        descriptorSet->writeChanges();
        cachedPrimitiveSets.add(descriptorSet);
    }
    Gfx::PRenderCommand renderCommand = graphics->createRenderCommand();    
    renderCommand->setViewport(target);
    for(uint32 i = 0; i < batch.elements.size(); ++i)
    {
        pipelineLayout->addDescriptorLayout(BasePass::INDEX_MATERIAL, material->getRenderMaterial()->getDescriptorLayout());
        pipelineLayout->create();
        descriptorSets[BasePass::INDEX_MATERIAL] = material->getDescriptor();
        descriptorSets[BasePass::INDEX_SCENE_DATA] = cachedPrimitiveSets[cachedPrimitiveIndex++];
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
    renderCommands.add(renderCommand);
}

Array<Gfx::PRenderCommand> BasePassMeshProcessor::getRenderCommands() 
{
    return renderCommands;
}

void BasePassMeshProcessor::clearCommands()
{
    renderCommands.clear();
    cachedPrimitiveSets.clear();
    cachedPrimitiveIndex = 0;
}

BasePass::BasePass(PRenderGraph renderGraph, const PScene scene, Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor source) 
    : RenderPass(renderGraph)
    , processor(new BasePassMeshProcessor(scene, viewport, graphics, false))
    , scene(scene)
    , graphics(graphics)
    , viewport(viewport)
    , descriptorSets(4)
    , source(source->getCameraComponent())
{
    UniformBufferCreateInfo uniformInitializer;
    basePassLayout = graphics->createPipelineLayout();

    lightLayout = graphics->createDescriptorLayout("LightLayout");
    lightLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    lightLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    lightLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_IMAGE);
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

void BasePass::beginFrame() 
{
    processor->clearCommands();
    primitiveLayout->reset();
    BulkResourceData uniformUpdate;

    viewParams.viewMatrix = source->getViewMatrix();
    viewParams.projectionMatrix = source->getProjectionMatrix();
    viewParams.cameraPosition = Vector4(source->getCameraPosition(), 0);
    viewParams.inverseProjectionMatrix = glm::inverse(viewParams.projectionMatrix);
    viewParams.screenDimensions = Vector2(static_cast<float>(viewport->getSizeX()), static_cast<float>(viewport->getSizeY()));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamBuffer->updateContents(uniformUpdate);
    descriptorSets[INDEX_VIEW_PARAMS]->updateBuffer(0, viewParamBuffer);
    descriptorSets[INDEX_VIEW_PARAMS]->writeChanges();
    for(auto &&meshBatch : scene->getStaticMeshes())
    {
        meshBatch.material->updateDescriptorData();
    }
}

void BasePass::render() 
{
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(0, scene->getLightBuffer());
    
	oLightIndexList->pipelineBarrier( 
		Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	oLightGrid->pipelineBarrier( 
		Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
		Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(1, oLightIndexList);
    descriptorSets[INDEX_LIGHT_ENV]->updateTexture(2, oLightGrid);
    descriptorSets[INDEX_LIGHT_ENV]->writeChanges();
    graphics->beginRenderPass(renderPass);
    for (auto &&meshBatch : scene->getStaticMeshes())
    {
        processor->addMeshBatch(meshBatch, renderPass, basePassLayout, primitiveLayout, descriptorSets);
    }
    graphics->executeCommands(processor->getRenderCommands());
    graphics->endRenderPass();
}

void BasePass::endFrame() 
{
}

void BasePass::publishOutputs() 
{
    colorAttachment = new Gfx::SwapchainAttachment(viewport->getOwner());
    renderGraph->registerRenderPassOutput("BASEPASS_COLOR", colorAttachment);
}

void BasePass::createRenderPass() 
{
    Gfx::PRenderTargetAttachment depthAttachment = renderGraph->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment->loadOp = Gfx::SE_ATTACHMENT_LOAD_OP_LOAD;
    Gfx::PRenderTargetLayout layout = new Gfx::RenderTargetLayout(colorAttachment, depthAttachment);
    renderPass = graphics->createRenderPass(layout);
    oLightIndexList = renderGraph->requestBuffer("LIGHTCULLING_OLIGHTLIST");
    oLightGrid = renderGraph->requestTexture("LIGHTCULLING_OLIGHTGRID");
}

void BasePass::modifyRenderPassMacros(Map<const char*, const char*>& defines)
{
    defines["INDEX_LIGHT_ENV"] = "0";
    defines["INDEX_VIEW_PARAMS"] = "1";
    defines["INDEX_MATERIAL"] = "2";
    defines["INDEX_SCENE_DATA"] = "3";
}
