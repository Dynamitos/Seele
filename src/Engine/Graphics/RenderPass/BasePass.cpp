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
        Gfx::PDescriptorSet descriptorSet = primitiveLayout->allocatedDescriptorSet();
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
    lightLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    uniformInitializer.resourceData.size = sizeof(LightEnv);
    uniformInitializer.resourceData.data = nullptr;
    uniformInitializer.bDynamic = true;
    lightUniform = graphics->createUniformBuffer(uniformInitializer);
    lightLayout->create();
    basePassLayout->addDescriptorLayout(INDEX_LIGHT_ENV, lightLayout);
    descriptorSets[INDEX_LIGHT_ENV] = lightLayout->allocatedDescriptorSet();

    viewLayout = graphics->createDescriptorLayout("ViewLayout");
    viewLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    uniformInitializer.resourceData.size = sizeof(ViewParameter);
    uniformInitializer.resourceData.data = (uint8*)&viewParams;
    uniformInitializer.bDynamic = true;
    viewParamBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    uniformInitializer.resourceData.size = sizeof(ScreenToViewParameter);
    uniformInitializer.resourceData.data = (uint8*)&screenToViewParams;
    uniformInitializer.bDynamic = true;
    screenToViewParamBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewLayout->create();
    basePassLayout->addDescriptorLayout(INDEX_VIEW_PARAMS, viewLayout);
    descriptorSets[INDEX_VIEW_PARAMS] = viewLayout->allocatedDescriptorSet();

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
    uniformUpdate.size = sizeof(LightEnv);
    uniformUpdate.data = (uint8*)&scene->getLightEnvironment();
    lightUniform->updateContents(uniformUpdate);
    descriptorSets[INDEX_LIGHT_ENV]->updateBuffer(0, lightUniform);
    descriptorSets[INDEX_LIGHT_ENV]->writeChanges();

    viewParams.viewMatrix = source->getViewMatrix();
    viewParams.projectionMatrix = source->getProjectionMatrix();
    viewParams.cameraPosition = Vector4(source->getCameraPosition(), 0);
    screenToViewParams.inverseProjectionMatrix = glm::inverse(viewParams.projectionMatrix);
    screenToViewParams.screenDimensions = Vector2(static_cast<float>(viewport->getSizeX()), static_cast<float>(viewport->getSizeY()));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamBuffer->updateContents(uniformUpdate);
    uniformUpdate.size = sizeof(ScreenToViewParameter);
    uniformUpdate.data = (uint8*)&screenToViewParams;
    screenToViewParamBuffer->updateContents(uniformUpdate);
    descriptorSets[INDEX_VIEW_PARAMS]->updateBuffer(0, viewParamBuffer);
    descriptorSets[INDEX_VIEW_PARAMS]->updateBuffer(1, screenToViewParamBuffer);
    descriptorSets[INDEX_VIEW_PARAMS]->writeChanges();
    for(auto &&meshBatch : scene->getStaticMeshes())
    {
        meshBatch.material->updateDescriptorData();
    }
}

void BasePass::render() 
{
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
}

void BasePass::modifyRenderPassMacros(Map<const char*, const char*>& defines)
{
    defines["INDEX_LIGHT_ENV"] = "0";
    defines["INDEX_VIEW_PARAMS"] = "1";
    defines["INDEX_MATERIAL"] = "2";
    defines["INDEX_SCENE_DATA"] = "3";
}
