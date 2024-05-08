#include "DepthPrepass.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include "Window/Window.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Actor/CameraActor.h"
#include "Math/Vector.h"
#include "RenderGraph.h"
#include "Graphics/Command.h"
#include "Graphics/StaticMeshVertexData.h"

using namespace Seele;

DepthPrepass::DepthPrepass(Gfx::PGraphics graphics, PScene scene) 
    : RenderPass(graphics, scene)
{
    depthPrepassLayout = graphics->createPipelineLayout("DepthPrepassLayout");
    depthPrepassLayout->addDescriptorLayout(viewParamsLayout);
    if (graphics->supportMeshShading())
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "DepthPass", "MeshletPass", false, false, "", true, true, "MeshletPass");
    }
    else
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "DepthPass", "LegacyPass");
    }
}

DepthPrepass::~DepthPrepass()
{   
}

void DepthPrepass::beginFrame(const Component::Camera& cam) 
{
    RenderPass::beginFrame(cam);
}

void DepthPrepass::render() 
{
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;
    
    // Others
    {
        Gfx::ShaderPermutation permutation;
        if (graphics->supportMeshShading())
        {
            permutation.setTaskFile("MeshletPass");
            permutation.setMeshFile("MeshletPass");
        }
        else
        {
            permutation.setVertexFile("LegacyPass");
        }
        for (VertexData* vertexData : VertexData::getList())
        {
            permutation.setVertexData(vertexData->getTypeName());
            const auto& materials = vertexData->getMaterialData();
            for (const auto& [_, materialData] : materials)
            {
                // Create Pipeline(VertexData)
                // Descriptors:
                // ViewData => global, static
                // VertexData => per meshtype
                // SceneData => per material instance
                permutation.setMaterial(materialData.material->getName());
                Gfx::PermutationId id(permutation);

                Gfx::ORenderCommand command = graphics->createRenderCommand("BaseRender");
                command->setViewport(viewport);

                const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
                assert(collection != nullptr);
                if (graphics->supportMeshShading())
                {
                    Gfx::MeshPipelineCreateInfo pipelineInfo;
                    pipelineInfo.taskShader = collection->taskShader;
                    pipelineInfo.meshShader = collection->meshShader;
                    pipelineInfo.fragmentShader = collection->fragmentShader;
                    pipelineInfo.pipelineLayout = collection->pipelineLayout;
                    pipelineInfo.renderPass = renderPass;
                    pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER;
                    pipelineInfo.multisampleState.samples = viewport->getSamples();
                    pipelineInfo.colorBlend.attachmentCount = 1;
                    Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                    command->bindPipeline(pipeline);
                }
                else
                {
                    Gfx::LegacyPipelineCreateInfo pipelineInfo;
                    pipelineInfo.vertexShader = collection->vertexShader;
                    pipelineInfo.fragmentShader = collection->fragmentShader;
                    pipelineInfo.pipelineLayout = collection->pipelineLayout;
                    pipelineInfo.renderPass = renderPass;
                    pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER;
                    pipelineInfo.multisampleState.samples = viewport->getSamples();
                    pipelineInfo.colorBlend.attachmentCount = 1;
                    Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                    command->bindPipeline(pipeline);
                }
                command->bindDescriptor(vertexData->getVertexDataSet());
                command->bindDescriptor(viewParamsSet);
                for (const auto& [_, instance] : materialData.instances)
                {
                    command->bindDescriptor(vertexData->getInstanceDataSet(), { instance.descriptorOffset, instance.descriptorOffset });
                    if (graphics->supportMeshShading())
                    {
                        command->drawMesh(vertexData->getMeshData(instance.meshId).size(), 1, 1);
                    }
                    else
                    {
                        command->bindIndexBuffer(vertexData->getIndexBuffer());
                        uint32 instanceOffset = 0;
                        for (const auto& meshData : vertexData->getMeshData(instance.meshId))
                        {
                            if (meshData.numIndices > 0)
                            {
                                command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, meshData.indicesOffset, instanceOffset);
                            }
                            instanceOffset++;
                        }
                    }
                }
                commands.add(std::move(command));
            }
        }
    }

    graphics->executeCommands(std::move(commands));
    graphics->endRenderPass();
}

void DepthPrepass::endFrame() 
{
}

void DepthPrepass::publishOutputs() 
{
}

void DepthPrepass::createRenderPass() 
{
}
