#include "StaticDepthPrepass.h"
#include "Graphics/StaticMeshVertexData.h"
#include "Graphics/Shader.h"

using namespace Seele;

StaticDepthPrepass::StaticDepthPrepass(Gfx::PGraphics graphics, PScene scene)
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

StaticDepthPrepass::~StaticDepthPrepass()
{
}

void StaticDepthPrepass::beginFrame(const Component::Camera& cam)
{
    RenderPass::beginFrame(cam);
}

void StaticDepthPrepass::render()
{
    // Static Meshes
    {
        Gfx::ShaderPermutation permutation;
        if (graphics->supportMeshShading())
        {
            permutation.setTaskFile("StaticMeshletPass");
            permutation.setMeshFile("StaticMeshletPass");
        }
        else
        {
            permutation.setVertexFile("LegacyPass");
        }
        StaticMeshVertexData* vd = StaticMeshVertexData::getInstance();
        permutation.setVertexData(vd->getTypeName());
        for (const auto& [_, mappings] : vd->)
        {
                permutation.setMaterial(mapping.material->getBaseMaterial()->getName());
                Gfx::PermutationId id(permutation);

                Gfx::ORenderCommand command = graphics->createRenderCommand("DepthRender");
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
                    pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER_OR_EQUAL;
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
                    pipelineInfo.depthStencilState.depthCompareOp = Gfx::SE_COMPARE_OP_GREATER_OR_EQUAL;
                    pipelineInfo.multisampleState.samples = viewport->getSamples();
                    pipelineInfo.colorBlend.attachmentCount = 1;
                    Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
                    command->bindPipeline(pipeline);
                }
                command->bindDescriptor(viewParamsSet);
                command->bindDescriptor(vd->getVertexDataSet());
                command->bindDescriptor(vd->getInstanceDataSet(), { 0, 0 });
                if (graphics->supportMeshShading())
                {
                    command->drawMesh(vd->getMeshData(mapping.mapped).size(), 1, 1);
                }
                else
                {
                    command->bindIndexBuffer(vd->getIndexBuffer());
                    uint32 instanceOffset = 0;
                    for (const auto& meshData : vd->getMeshData(mapping.mapped))
                    {
                        if (meshData.numIndices > 0)
                        {
                            command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, meshData.indicesOffset, instanceOffset);
                        }
                        instanceOffset++;
                    }
                }

                commands.add(std::move(command));
            }
        }
    }
}

void StaticDepthPrepass::endFrame()
{
}

void StaticDepthPrepass::publishOutputs()
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

void StaticDepthPrepass::createRenderPass()
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
