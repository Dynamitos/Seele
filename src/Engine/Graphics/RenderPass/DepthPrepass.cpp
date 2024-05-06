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

using namespace Seele;

DepthPrepass::DepthPrepass(Gfx::PGraphics graphics, PScene scene) 
    : RenderPass(graphics, scene)
    , descriptorSets(3)
{
    depthPrepassLayout = graphics->createPipelineLayout("DepthPrepassLayout");
    depthPrepassLayout->addDescriptorLayout(viewParamsLayout);
    if (graphics->supportMeshShading())
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "DepthPass", "MeshletBasePass", false, false, "", true, true, "MeshletBasePass");
    }
    else
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "DepthPass", "LegacyBasePass");
    }
}

DepthPrepass::~DepthPrepass()
{   
}

void DepthPrepass::beginFrame(const Component::Camera& cam) 
{
    RenderPass::beginFrame(cam);
    descriptorSets[INDEX_VIEW_PARAMS] = viewParamsSet;
}

void DepthPrepass::render() 
{
    Gfx::ShaderPermutation permutation;
    if(graphics->supportMeshShading())
    {
        permutation.setTaskFile("MeshletBasePass");
        permutation.setMeshFile("MeshletBasePass");
    }
    else
    {
        permutation.setVertexFile("LegacyBasePass");
    }
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;
    for (VertexData* vertexData : VertexData::getList())
    {
        permutation.setVertexData(vertexData->getTypeName());
        const auto& materials = vertexData->getMaterialData();
        for (const auto& [_, materialData] : materials) 
        {
            // Create Pipeline(Material, VertexData)
            // Descriptors:
            // ViewData => global, static
            // Material => per material
            // VertexData => per meshtype
            // SceneData => per material instance
            Gfx::PermutationId id(permutation);
            
            Gfx::ORenderCommand command = graphics->createRenderCommand("DepthRender");
            command->setViewport(viewport);
            
            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
            assert(collection != nullptr);
            if(graphics->supportMeshShading())
            {
                Gfx::MeshPipelineCreateInfo pipelineInfo;
                pipelineInfo.taskShader = collection->taskShader;
                pipelineInfo.meshShader = collection->meshShader;
                pipelineInfo.fragmentShader = collection->fragmentShader;
                pipelineInfo.pipelineLayout = collection->pipelineLayout;
                pipelineInfo.renderPass = renderPass;
                pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER;
                pipelineInfo.multisampleState.samples = viewport->getSamples();
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
                //pipelineInfo.depthStencilState.depthWriteEnable = false;
                pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER;
                pipelineInfo.multisampleState.samples = viewport->getSamples();
                Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                command->bindPipeline(pipeline);
            }

            descriptorSets[INDEX_VERTEX_DATA] = vertexData->getVertexDataSet();
            for (const auto& [_, instance] : materialData.instances)
            {
                //descriptorSets[INDEX_MATERIAL] = instance.materialInstance->getDescriptorSet();
                descriptorSets[INDEX_SCENE_DATA] = instance.descriptorSet;
                command->bindDescriptor(descriptorSets);
                if (graphics->supportMeshShading())
                {
                    command->drawMesh(instance.meshes.size(), 1, 1);
                }
                else
                {
                    command->bindIndexBuffer(vertexData->getIndexBuffer());
                    uint32 instanceOffset = 0;
                    for (const auto& [_, meshData] : instance.meshes)
                    {
                        if (meshData.numIndices > 0)
                        {
                            command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, meshData.indicesOffset, instanceOffset++);
                        }
                    }
                }
            }
            commands.add(std::move(command));
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
    // If we render to a part of an image, the depth buffer itself must
    // still match the size of the whole image or their coordinate systems go out of sync
    TextureCreateInfo depthBufferInfo = {
        .format = Gfx::SE_FORMAT_D32_SFLOAT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };
    depthBuffer = graphics->createTexture2D(depthBufferInfo);
    depthAttachment = 
        Gfx::RenderTargetAttachment(depthBuffer,
            Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_GENERAL,
            Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("DEPTHPREPASS_DEPTH", depthAttachment);
}

void DepthPrepass::createRenderPass() 
{
    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .depthAttachment = depthAttachment,
    };
    Array<Gfx::SubPassDependency> dependency = {
        {
            .srcSubpass = ~0U,
            .dstSubpass = 0,
            .srcStage = Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_SHADER_READ_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        }
    };
    renderPass = graphics->createRenderPass(std::move(layout), dependency, viewport);
}

void DepthPrepass::modifyRenderPassMacros(Map<const char*, const char*>&) 
{
}
