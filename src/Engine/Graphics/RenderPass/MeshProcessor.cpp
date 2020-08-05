#include "MeshProcessor.h"
#include "Graphics/Graphics.h"
#include "Graphics/VertexShaderInput.h"

using namespace Seele;

MeshProcessor::MeshProcessor(const PScene scene, Gfx::PGraphics graphics)
    : scene(scene)
    , graphics(graphics)
{

}

MeshProcessor::~MeshProcessor()
{
}

void MeshProcessor::buildMeshDrawCommand(
    const MeshBatch& meshBatch, 
    const PPrimitiveComponent primitiveComponent,
    const Gfx::PRenderPass renderPass,
    Gfx::PRenderCommand drawCommand,
    PMaterial material, 
    Gfx::PVertexShader vertexShader,
    Gfx::PControlShader controlShader,
    Gfx::PEvaluationShader evaluationShader,
    Gfx::PGeometryShader geometryShader,
    Gfx::PFragmentShader fragmentShader,
    bool positionOnly)
{
    const PVertexShaderInput vertexInput = meshBatch.vertexInput;

    GraphicsPipelineCreateInfo pipelineInitializer;
    pipelineInitializer.topology = meshBatch.topology;
    Gfx::PVertexDeclaration vertexDecl = positionOnly ? vertexInput->getPositionDeclaration() : vertexInput->getDeclaration();
    pipelineInitializer.vertexDeclaration = vertexDecl;
    pipelineInitializer.vertexShader = vertexShader;
    pipelineInitializer.controlShader = controlShader;
    pipelineInitializer.evalShader = evaluationShader;
    pipelineInitializer.geometryShader = geometryShader;
    pipelineInitializer.fragmentShader = fragmentShader;
    pipelineInitializer.renderPass = renderPass;

    VertexInputStreamArray vertexStreams;
    if(positionOnly)
    {
        vertexInput->getPositionOnlyStream(vertexStreams);
    }
    else
    {
        vertexInput->getStreams(vertexStreams);
    }
    drawCommand->bindVertexBuffer(vertexStreams);
    Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(pipelineInitializer);
    drawCommand->bindPipeline(pipeline);
    for(auto element : meshBatch.elements)
    {
        drawCommand->bindIndexBuffer(element.indexBuffer);
        drawCommand->draw(element);
    }
}