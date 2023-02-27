#include "DepthPrepass.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Component/Camera.h"
#include "Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Material/MaterialInterface.h"

using namespace Seele;

DepthPrepassMeshProcessor::DepthPrepassMeshProcessor(Gfx::PGraphics graphics) 
    : MeshProcessor(graphics)
{
}

DepthPrepassMeshProcessor::~DepthPrepassMeshProcessor() 
{ 
}

void DepthPrepassMeshProcessor::processMeshBatch(
    const MeshBatch& batch, 
    Gfx::PViewport target,
    Gfx::PRenderPass renderPass,
    Gfx::PPipelineLayout baseLayout,
    Gfx::PDescriptorLayout primitiveLayout,
    Array<Gfx::PDescriptorSet> descriptorSets,
    int32 /*staticMeshId*/) 
{
    //std::cout << "Depth void started" << std::endl;
    PMaterialInterface material = batch.material;
    //const Gfx::MaterialShadingModel shadingModel = material->getShadingModel();

    const PVertexShaderInput vertexInput = batch.vertexInput;

	const Gfx::ShaderCollection* collection = material->getShaders(Gfx::RenderPassType::DepthPrepass, vertexInput->getType());
    if (collection == nullptr)
    {
        material->createShaders(graphics, Gfx::RenderPassType::DepthPrepass, vertexInput->getType());
        collection = material->getShaders(Gfx::RenderPassType::DepthPrepass, vertexInput->getType());
    }
    assert(collection != nullptr);
    
    Gfx::PRenderCommand renderCommand = graphics->createRenderCommand();    
    renderCommand->setViewport(target);
    Gfx::PPipelineLayout pipelineLayout = graphics->createPipelineLayout(baseLayout);
    pipelineLayout->addDescriptorLayout(DepthPrepass::INDEX_MATERIAL, material->getDescriptorLayout());
    pipelineLayout->create();
    Gfx::PDescriptorSet materialSet = material->createDescriptorSet();
    descriptorSets[DepthPrepass::INDEX_MATERIAL] = materialSet;
    for(uint32 i = 0; i < batch.elements.size(); ++i)
    {
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
    std::scoped_lock lock(commandLock);
    renderCommands.add(renderCommand);
    //std::cout << "Finished depth job" << std::endl;
    //co_return;
}

DepthPrepass::DepthPrepass(Gfx::PGraphics graphics) 
    : RenderPass(graphics)
    , processor(new DepthPrepassMeshProcessor(graphics))
    , descriptorSets(3)
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
    primitiveLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    primitiveLayout->create();
    depthPrepassLayout->addDescriptorLayout(INDEX_SCENE_DATA, primitiveLayout);
    depthPrepassLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = (Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT),
        .offset = 0,
        .size = sizeof(uint32),
    });
}

DepthPrepass::~DepthPrepass()
{   
}

void DepthPrepass::beginFrame(const Component::Camera& cam) 
{
    processor->clearCommands();
    primitiveLayout->reset();
    BulkResourceData uniformUpdate;

    viewParams.viewMatrix = cam.getViewMatrix();
    viewParams.projectionMatrix = viewport->getProjectionMatrix();
    viewParams.cameraPosition = Vector4(cam.getCameraPosition(), 1);
    viewParams.screenDimensions = Vector2(static_cast<float>(viewport->getSizeX()), static_cast<float>(viewport->getSizeY()));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamBuffer->updateContents(uniformUpdate);
    viewLayout->reset();
    descriptorSets[INDEX_SCENE_DATA] = primitiveLayout->allocateDescriptorSet();
    descriptorSets[INDEX_SCENE_DATA]->updateBuffer(0, passData.sceneDataBuffer);
    descriptorSets[INDEX_SCENE_DATA]->writeChanges();
    descriptorSets[INDEX_VIEW_PARAMS] = viewLayout->allocateDescriptorSet();
    descriptorSets[INDEX_VIEW_PARAMS]->updateBuffer(0, viewParamBuffer);
    descriptorSets[INDEX_VIEW_PARAMS]->writeChanges();
    //std::cout << "DepthPrepass beginFrame()" << std::endl;
    //co_return;
}

void DepthPrepass::render() 
{
    depthAttachment->getTexture()->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    depthAttachment->getTexture()->transferOwnership(Gfx::QueueType::GRAPHICS);
    graphics->beginRenderPass(renderPass);
    for (const auto& meshBatch : passData.staticDrawList)
    {
        processor->processMeshBatch(meshBatch, viewport, renderPass, depthPrepassLayout, primitiveLayout, descriptorSets);
    }
    graphics->executeCommands(processor->getRenderCommands());
    graphics->endRenderPass();
    //std::cout << "DepthPrepass render()" << std::endl;
    //co_return;
}

void DepthPrepass::endFrame() 
{
    //std::cout << "DepthPrepass endFrame()" << std::endl;
    //co_return;
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
