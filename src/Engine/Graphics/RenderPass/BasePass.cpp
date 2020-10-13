#include "BasePass.h"
#include "Graphics/Graphics.h"
#include "Graphics/Window.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Actor/CameraActor.h"

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
    int32 staticMeshId) 
{
    const PMaterialAsset material = batch.material;
    const Gfx::MaterialShadingModel shadingModel = material->getShadingModel();

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
        pipelineLayout->addDescriptorLayout(2, material->getRenderMaterial()->getDescriptorLayout());
        pipelineLayout->create();
        descriptorSets[2] = material->getDescriptor();
        descriptorSets[3] = cachedPrimitiveSets[cachedPrimitiveIndex++];
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

BasePass::BasePass(const PScene scene, Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor source) 
    : processor(new BasePassMeshProcessor(scene, viewport, graphics, false))
    , scene(scene)
    , graphics(graphics)
    , viewport(viewport)
    , descriptorSets(4)
    , source(source->getCameraComponent())
{
    Gfx::PRenderTargetAttachment colorAttachment = new Gfx::SwapchainAttachment(viewport->getOwner());
    TextureCreateInfo depthBufferInfo;
    depthBufferInfo.width = viewport->getSizeX();
    depthBufferInfo.height = viewport->getSizeY();
    depthBufferInfo.format = Gfx::SE_FORMAT_D32_SFLOAT;
    depthBufferInfo.usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    Gfx::PRenderTargetAttachment depthAttachment = 
        new Gfx::RenderTargetAttachment(depthBuffer, Gfx::SE_ATTACHMENT_LOAD_OP_DONT_CARE, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    Gfx::PRenderTargetLayout layout = new Gfx::RenderTargetLayout(colorAttachment, depthAttachment);
    renderPass = graphics->createRenderPass(layout);

    BulkResourceData uniformInitializer;

    basePassLayout = graphics->createPipelineLayout();

    lightLayout = graphics->createDescriptorLayout();
    lightLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    uniformInitializer.size = sizeof(LightEnv);
    uniformInitializer.data = (uint8*)&scene->getLightEnvironment();
    lightUniform = graphics->createUniformBuffer(uniformInitializer);
    lightLayout->create();
    basePassLayout->addDescriptorLayout(0, lightLayout);
    descriptorSets[0] = lightLayout->allocatedDescriptorSet();

    viewLayout = graphics->createDescriptorLayout();
    viewLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    uniformInitializer.size = sizeof(ViewParameter);
    uniformInitializer.data = (uint8*)&viewParams;
    viewParamBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    uniformInitializer.size = sizeof(ScreenToViewParameter);
    uniformInitializer.data = (uint8*)&screenToViewParams;
    screenToViewParamBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewLayout->create();
    basePassLayout->addDescriptorLayout(1, viewLayout);
    descriptorSets[1] = viewLayout->allocatedDescriptorSet();

    primitiveLayout = graphics->createDescriptorLayout();
    primitiveLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    primitiveLayout->create();
    basePassLayout->addDescriptorLayout(3, primitiveLayout);
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
    descriptorSets[0]->updateBuffer(0, lightUniform);
    descriptorSets[0]->writeChanges();

    viewParams.viewMatrix = source->getViewMatrix();
    viewParams.projectionMatrix = source->getProjectionMatrix();
    viewParams.cameraPosition = Vector4(source->getTransform().getPosition(), 0);
    screenToViewParams.inverseProjectionMatrix = glm::inverse(viewParams.projectionMatrix);
    screenToViewParams.screenDimensions = Vector2(static_cast<float>(viewport->getSizeX()), static_cast<float>(viewport->getSizeY()));
    uniformUpdate.size = sizeof(ViewParameter);
    uniformUpdate.data = (uint8*)&viewParams;
    viewParamBuffer->updateContents(uniformUpdate);
    uniformUpdate.size = sizeof(ScreenToViewParameter);
    uniformUpdate.data = (uint8*)&screenToViewParams;
    screenToViewParamBuffer->updateContents(uniformUpdate);
    descriptorSets[1]->updateBuffer(0, viewParamBuffer);
    descriptorSets[1]->updateBuffer(1, screenToViewParamBuffer);
    descriptorSets[1]->writeChanges();
}

void BasePass::render() 
{
    graphics->beginRenderPass(renderPass);
    for (auto &&primitive : scene->getStaticMeshes())
    {
        processor->addMeshBatch(primitive, renderPass, basePassLayout, primitiveLayout, descriptorSets);
    }
    graphics->executeCommands(processor->getRenderCommands());
    graphics->endRenderPass();
}

void BasePass::endFrame() 
{
}
