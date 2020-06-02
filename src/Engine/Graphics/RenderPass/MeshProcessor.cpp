#include "MeshProcessor.h"
#include "Graphics/Graphics.h"
#include "VertexFactory.h"

using namespace Seele;

MeshProcessor::MeshProcessor(const PScene scene, Gfx::PGraphics graphics)
    : scene(scene)
    , graphics(graphics)
{

}

MeshProcessor::~MeshProcessor()
{
}

void MeshProcessor::buildMeshDrawCommands(
    const MeshBatch& meshBatch, 
    const PPrimitiveComponent primitiveComponent, 
    PMaterial material, 
    Gfx::PVertexShader vertexShader,
    Gfx::PControlShader controlShader,
    Gfx::PEvaluationShader evaluationShader,
    Gfx::PGeometryShader geometryShader,
    Gfx::PFragmentShader fragmentShader,
    bool positionOnly)
{
    const PVertexFactory vertexFactory = meshBatch.vertexFactory;

    GraphicsPipelineCreateInfo pipelineInitializer;
    pipelineInitializer.topology = meshBatch.topology;
    Gfx::PVertexDeclaration vertexDecl = positionOnly ? vertexFactory->getPositionDeclaration() : vertexFactory->getDeclaration();
    pipelineInitializer.vertexDeclaration = vertexDecl;
    pipelineInitializer.vertexShader = vertexShader;
    pipelineInitializer.controlShader = controlShader;
    pipelineInitializer.evalShader = evaluationShader;
    pipelineInitializer.geometryShader = geometryShader;
    pipelineInitializer.fragmentShader = fragmentShader;

    VertexInputStreamArray vertexStreams;
    if(positionOnly)
    {
        vertexFactory->getPositionOnlyStream(vertexStreams);
    }
    else
    {
        vertexFactory->getStreams(vertexStreams);
    }
    if(vertexShader != nullptr)
    {
        
    }    
}