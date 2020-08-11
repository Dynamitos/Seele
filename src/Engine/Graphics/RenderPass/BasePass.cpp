#include "BasePass.h"
#include "Graphics/Graphics.h"
#include "Graphics/Window.h"

using namespace Seele;

BasePassMeshProcessor::BasePassMeshProcessor(const PScene scene, Gfx::PGraphics graphics, uint8 translucentBasePass) 
    : MeshProcessor(scene, graphics)
    , translucentBasePass(translucentBasePass)
{
}

BasePassMeshProcessor::~BasePassMeshProcessor() 
{ 
}

void BasePassMeshProcessor::addMeshBatch(
    const MeshBatch& batch, 
    const PPrimitiveComponent primitiveComponent,
    const Gfx::PRenderPass renderPass,
    int32 staticMeshId) 
{
    const PMaterial material = batch.material;
    const Gfx::MaterialShadingModel shadingModel = material->getShadingModel();

    const PVertexShaderInput vertexInput = batch.vertexInput;

    //TODO query tesselation

	const Gfx::ShaderCollection* collection = material->getShaders(Gfx::RenderPassType::BasePass, vertexInput);
    Gfx::PRenderCommand renderCommand = graphics->createRenderCommand();
    buildMeshDrawCommand(batch, 
        primitiveComponent, 
        renderPass, 
        renderCommand, 
        material, 
        collection->vertexShader, 
        collection->controlShader,
        collection->evalutionShader,
        collection->geometryShader,
        collection->fragmentShader,
        false);
    renderCommands.add(renderCommand);
}

Array<Gfx::PRenderCommand> BasePassMeshProcessor::getRenderCommands() 
{
    return renderCommands;
}

void BasePassMeshProcessor::clearCommands()
{
    renderCommands.clear();
}

BasePass::BasePass(const PScene scene, Gfx::PGraphics graphics, Gfx::PViewport viewport) 
    : processor(new BasePassMeshProcessor(scene, graphics, false))
    , scene(scene)
    , graphics(graphics)
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
}

BasePass::~BasePass() 
{   
}

void BasePass::render() 
{
    processor->clearCommands();
    graphics->beginRenderPass(renderPass);
    for (auto &&primitive : scene->getPrimitives())
    {
        for (auto &&meshBatch : primitive->staticMeshes)
        {
            processor->addMeshBatch(meshBatch, primitive, renderPass);
        }
    }
    graphics->executeCommands(processor->getRenderCommands());
    graphics->endRenderPass();
}
