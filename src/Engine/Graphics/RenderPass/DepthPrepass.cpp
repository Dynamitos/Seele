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
    depthPrepassLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT,
        .offset = 0,
        .size = sizeof(VertexData::DrawCallOffsets),
        });
    if (graphics->supportMeshShading())
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "DepthPass", "MeshletPass", false, false, "", true, true, "ViewCullingTask");
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

    Gfx::ShaderPermutation permutation;
    if (graphics->supportMeshShading())
    {
        permutation.setTaskFile("ViewCullingTask");
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
        for (const auto& materialData : materials)
        {
            // material not used for any active meshes, skip
            if (materialData.instances.size() == 0)
                continue;
            // Create Pipeline(VertexData)
            // Descriptors:
            // ViewData => global, static
            // VertexData => per meshtype
            // SceneData => per meshtype
            Gfx::PermutationId id(permutation);

            Gfx::ORenderCommand command = graphics->createRenderCommand("DepthRender");
            command->setViewport(viewport);

            const Gfx::ShaderCollection* collection = graphics->getShaderCompiler()->findShaders(id);
            assert(collection != nullptr);
            if (graphics->supportMeshShading())
            {
                Gfx::MeshPipelineCreateInfo pipelineInfo = {
                    .taskShader = collection->taskShader,
                    .meshShader = collection->meshShader,
                    .fragmentShader = collection->fragmentShader,
                    .renderPass = renderPass,
                    .pipelineLayout = collection->pipelineLayout,
                    .multisampleState = {
                        .samples = viewport->getSamples(),},
                    .depthStencilState = {
                        .depthCompareOp = Gfx::SE_COMPARE_OP_GREATER,
                    },
                    .colorBlend = {
                        .attachmentCount = 1,
                    },
                };
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
            for (const auto& drawCall : materialData.instances)
            {
                command->bindDescriptor(vertexData->getInstanceDataSet());
                command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT, 0, sizeof(VertexData::DrawCallOffsets), &drawCall.offsets);
                if (graphics->supportMeshShading())
                {
                    command->drawMesh(drawCall.numMeshes, 1, 1);
                }
                else
                {
                    //command->bindIndexBuffer(vertexData->getIndexBuffer());
                    //uint32 instanceOffset = 0;
                    //for (const auto& meshData : vertexData->getMeshData(instance.meshId))
                    //{
                    //    if (meshData.numIndices > 0)
                    //    {
                    //        command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, meshData.indicesOffset, instanceOffset);
                    //    }
                    //    instanceOffset++;
                    //}
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
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), viewport);
}
