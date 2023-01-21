#include "MeshProcessor.h"
#include "Graphics/Graphics.h"
#include "Graphics/VertexShaderInput.h"
#include "Graphics/Vulkan/VulkanGraphicsResources.h"

using namespace Seele;

MeshProcessor::MeshProcessor(Gfx::PGraphics graphics)
    : graphics(graphics)
{

}

MeshProcessor::~MeshProcessor()
{
}

void MeshProcessor::buildMeshDrawCommand(
    const MeshBatch& meshBatch, 
//        const PPrimitiveComponent primitiveComponent,
    const Gfx::PRenderPass renderPass,
    Gfx::PPipelineLayout pipelineLayout,
    Gfx::PRenderCommand drawCommand,
    const Array<Gfx::PDescriptorSet>& descriptors, 
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
    pipelineInitializer.vertexShader = vertexShader;
    pipelineInitializer.controlShader = controlShader;
    pipelineInitializer.evalShader = evaluationShader;
    pipelineInitializer.geometryShader = geometryShader;
    pipelineInitializer.fragmentShader = fragmentShader;
    pipelineInitializer.renderPass = renderPass;
    pipelineInitializer.pipelineLayout = pipelineLayout;

    VertexInputStreamArray vertexStreams;
    if(positionOnly)
    {
        vertexInput->getPositionOnlyStream(vertexStreams);
        pipelineInitializer.vertexDeclaration = vertexInput->getPositionDeclaration();
    }
    else
    {
        vertexInput->getStreams(vertexStreams);
        pipelineInitializer.vertexDeclaration = vertexInput->getDeclaration();
    }
    Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(pipelineInitializer);
    drawCommand->bindPipeline(pipeline);
    drawCommand->bindVertexBuffer(vertexStreams);
    drawCommand->bindDescriptor(descriptors);
    for(const auto& element : meshBatch.elements)
    {
        drawCommand->pushConstants(pipelineLayout, Gfx::SE_SHADER_STAGE_VERTEX_BIT | Gfx::SE_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32), &element.sceneDataIndex);
        if(element.indexBuffer != nullptr)
        {
            drawCommand->bindIndexBuffer(element.indexBuffer);
            drawCommand->draw(element);
        }
        else
        {
            drawCommand->draw(vertexStreams[0].vertexBuffer->getNumVertices(), 1, 0, 0);
        }
    }
}

Array<Gfx::PRenderCommand> MeshProcessor::getRenderCommands() 
{
    return renderCommands;
}

void MeshProcessor::clearCommands()
{
    renderCommands.clear();
}
