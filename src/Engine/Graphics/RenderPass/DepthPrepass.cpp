#include "DepthPrepass.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"

using namespace Seele;

DepthPrepassMeshProcessor::DepthPrepassMeshProcessor(Gfx::PViewport viewport, Gfx::PGraphics graphics) 
    : MeshProcessor(graphics)
    , target(viewport)
{
}

DepthPrepassMeshProcessor::~DepthPrepassMeshProcessor() 
{ 
}

Job DepthPrepassMeshProcessor::processMeshBatch(
    const MeshBatch& batch, 
//    const PPrimitiveComponent primitiveComponent,
    const Gfx::PRenderPass& renderPass,
    Gfx::PPipelineLayout pipelineLayout,
    Gfx::PDescriptorLayout primitiveLayout,
    Array<Gfx::PDescriptorSet> descriptorSets,
    int32 /*staticMeshId*/) 
{
    std::cout << "Depth job started" << std::endl;
    PMaterialAsset material = batch.material;
    //const Gfx::MaterialShadingModel shadingModel = material->getShadingModel();

    const PVertexShaderInput vertexInput = batch.vertexInput;

	const Gfx::ShaderCollection* collection = material->getShaders(Gfx::RenderPassType::DepthPrepass, vertexInput->getType());
    assert(collection != nullptr);
    
    Gfx::PRenderCommand renderCommand = graphics->createRenderCommand();    
    renderCommand->setViewport(target);
    pipelineLayout->addDescriptorLayout(DepthPrepass::INDEX_MATERIAL, material->getDescriptorLayout());
    pipelineLayout->create();
    Gfx::PDescriptorSet materialSet = material->createDescriptorSet();
    descriptorSets[DepthPrepass::INDEX_MATERIAL] = materialSet;
    for(uint32 i = 0; i < batch.elements.size(); ++i)
    {   
        Gfx::PDescriptorSet descriptorSet = primitiveLayout->allocateDescriptorSet();
        descriptorSet->updateBuffer(0, batch.elements[i].uniformBuffer);
        descriptorSet->writeChanges();
        descriptorSets[DepthPrepass::INDEX_SCENE_DATA] = descriptorSet;
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
    std::unique_lock lock(commandLock);
    renderCommands.add(renderCommand);
    std::cout << "Finished depth job" << std::endl;
    co_return;
}

DepthPrepass::DepthPrepass(Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor source) 
    : RenderPass(graphics, viewport)
    , processor(new DepthPrepassMeshProcessor(viewport, graphics))
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
    viewLayout->create();
    depthPrepassLayout->addDescriptorLayout(INDEX_VIEW_PARAMS, viewLayout);

    primitiveLayout = graphics->createDescriptorLayout("PrimitiveLayout");
    primitiveLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    primitiveLayout->create();
    depthPrepassLayout->addDescriptorLayout(INDEX_SCENE_DATA, primitiveLayout);
}

DepthPrepass::~DepthPrepass()
{   
}

MainJob DepthPrepass::beginFrame() 
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
    viewLayout->reset();
    descriptorSets[INDEX_VIEW_PARAMS] = viewLayout->allocateDescriptorSet();
    descriptorSets[INDEX_VIEW_PARAMS]->updateBuffer(0, viewParamBuffer);
    descriptorSets[INDEX_VIEW_PARAMS]->writeChanges();
    co_return;
}

MainJob DepthPrepass::render() 
{
    depthAttachment->getTexture()->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    depthAttachment->getTexture()->transferOwnership(Gfx::QueueType::GRAPHICS);
    graphics->beginRenderPass(renderPass);
    List<Job> jobs;
    for (auto &&meshBatch : passData.staticDrawList)
    {
        jobs.add(processor->processMeshBatch(meshBatch, renderPass, depthPrepassLayout, primitiveLayout, descriptorSets));
    }
    co_await Job::all(jobs);
    std::cout << "Finished waiting depth jobs " << std::endl;
    graphics->executeCommands(processor->getRenderCommands());
    graphics->endRenderPass();
    co_return;
}

MainJob DepthPrepass::endFrame() 
{
    co_return;
}

void DepthPrepass::publishOutputs() 
{
    TextureCreateInfo depthBufferInfo;
    // If we render to a part of an image, the depth buffer itself must
    // still match the size of the whole image or their coordinate systems go out of sync
    depthBufferInfo.width = viewport->getOwner()->getSizeX();
    depthBufferInfo.height = viewport->getOwner()->getSizeY();
    depthBufferInfo.format = Gfx::SE_FORMAT_D32_SFLOAT;
    depthBufferInfo.usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment = 
        new Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    depthAttachment->clear.depthStencil.depth = 1.0f;
    resources->registerRenderPassOutput("DEPTHPREPASS_DEPTH", depthAttachment);
}

void DepthPrepass::createRenderPass() 
{
    Gfx::PRenderTargetLayout layout = new Gfx::RenderTargetLayout(depthAttachment);
    renderPass = graphics->createRenderPass(layout, viewport);
}

void DepthPrepass::modifyRenderPassMacros(Map<const char*, const char*>& defines) 
{
    defines["INDEX_VIEW_PARAMS"] = "0";
    defines["INDEX_MATERIAL"] = "1";
    defines["INDEX_SCENE_DATA"] = "2";
}
