#include "DepthPrepass.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"

using namespace Seele;

DepthPrepassMeshProcessor::DepthPrepassMeshProcessor(const PScene scene, Gfx::PViewport viewport, Gfx::PGraphics graphics) 
    : MeshProcessor(scene, graphics)
    , target(viewport)
    , cachedPrimitiveIndex(0)
{
}

DepthPrepassMeshProcessor::~DepthPrepassMeshProcessor() 
{ 
}

void DepthPrepassMeshProcessor::addMeshBatch(
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

	const Gfx::ShaderCollection* collection = material->getRenderMaterial()->getShaders(Gfx::RenderPassType::DepthPrepass, vertexInput->getType());
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
        pipelineLayout->addDescriptorLayout(DepthPrepass::INDEX_MATERIAL, material->getRenderMaterial()->getDescriptorLayout());
        pipelineLayout->create();
        descriptorSets[DepthPrepass::INDEX_MATERIAL] = material->getDescriptor();
        descriptorSets[DepthPrepass::INDEX_SCENE_DATA] = cachedPrimitiveSets[cachedPrimitiveIndex++];
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
            true);
    }
    renderCommands.add(renderCommand);
}

Array<Gfx::PRenderCommand> DepthPrepassMeshProcessor::getRenderCommands() 
{
    return renderCommands;
}

void DepthPrepassMeshProcessor::clearCommands()
{
    renderCommands.clear();
    cachedPrimitiveSets.clear();
    cachedPrimitiveIndex = 0;
}

DepthPrepass::DepthPrepass(PRenderGraph renderGraph, const PScene scene, Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor source) 
    : RenderPass(renderGraph)
    , processor(new DepthPrepassMeshProcessor(scene, viewport, graphics))
    , scene(scene)
    , graphics(graphics)
    , viewport(viewport)
    , descriptorSets(3)
    , source(source->getCameraComponent())
{
    UniformBufferCreateInfo uniformInitializer;

    depthPrepassLayout = graphics->createPipelineLayout();

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
    depthPrepassLayout->addDescriptorLayout(INDEX_VIEW_PARAMS, viewLayout);
    descriptorSets[INDEX_VIEW_PARAMS] = viewLayout->allocatedDescriptorSet();

    primitiveLayout = graphics->createDescriptorLayout("PrimitiveLayout");
    primitiveLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    primitiveLayout->create();
    depthPrepassLayout->addDescriptorLayout(INDEX_SCENE_DATA, primitiveLayout);
}

DepthPrepass::~DepthPrepass()
{   
}

void DepthPrepass::beginFrame() 
{
    processor->clearCommands();
    primitiveLayout->reset();
    BulkResourceData uniformUpdate;

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

void DepthPrepass::render() 
{
    graphics->beginRenderPass(renderPass);
    for (auto &&meshBatch : scene->getStaticMeshes())
    {
        processor->addMeshBatch(meshBatch, renderPass, depthPrepassLayout, primitiveLayout, descriptorSets);
    }
    graphics->executeCommands(processor->getRenderCommands());
    graphics->endRenderPass();
}

void DepthPrepass::endFrame() 
{
}

void DepthPrepass::publishOutputs() 
{
    TextureCreateInfo depthBufferInfo;
    depthBufferInfo.width = viewport->getSizeX();
    depthBufferInfo.height = viewport->getSizeY();
    depthBufferInfo.format = Gfx::SE_FORMAT_D32_SFLOAT;
    depthBufferInfo.usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment = 
        new Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    depthAttachment->clear.depthStencil.depth = 1.0f;
    renderGraph->registerRenderPassOutput("DEPTHPREPASS_DEPTH", depthAttachment);
}

void DepthPrepass::createRenderPass() 
{
    Gfx::PRenderTargetLayout layout = new Gfx::RenderTargetLayout(depthAttachment);
    renderPass = graphics->createRenderPass(layout);
}

void DepthPrepass::modifyRenderPassMacros(Map<const char*, const char*>& defines) 
{
    defines["INDEX_VIEW_PARAMS"] = "0";
    defines["INDEX_MATERIAL"] = "1";
    defines["INDEX_SCENE_DATA"] = "2";
}
