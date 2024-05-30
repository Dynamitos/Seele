#include "CachedDepthPass.h"
#include "Graphics/Shader.h"

using namespace Seele;

extern bool usePositionOnly;
extern bool useViewCulling;

CachedDepthPass::CachedDepthPass(Gfx::PGraphics graphics, PScene scene)
    : RenderPass(graphics, scene)
{
    depthPrepassLayout = graphics->createPipelineLayout("CachedDepthLayout");
    depthPrepassLayout->addDescriptorLayout(viewParamsLayout);
    depthPrepassLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(VertexData::DrawCallOffsets),
    });
    if (graphics->supportMeshShading())
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "CachedDepthPass", "VisibilityMeshletPass", false, true, "VisibilityPass", true, true, "DrawListTask");
    }
    else
    {
        graphics->getShaderCompiler()->registerRenderPass(depthPrepassLayout, "CachedDepthPass", "LegacyPass");
    }
}

CachedDepthPass::~CachedDepthPass()
{
}

void CachedDepthPass::beginFrame(const Component::Camera &cam)
{
    RenderPass::beginFrame(cam);
}

void CachedDepthPass::render()
{
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;

    Gfx::ShaderPermutation permutation;
    permutation.setPositionOnly(usePositionOnly);
    permutation.setViewCulling(useViewCulling);
    if (graphics->supportMeshShading())
    {
        permutation.setTaskFile("DrawListTask");
        permutation.setMeshFile("VisibilityMeshletPass");
    }
    else
    {
        permutation.setVertexFile("LegacyPass");
    }
    permutation.setFragmentFile("VisibilityPass");
    for (VertexData *vertexData : VertexData::getList())
    {
        permutation.setVertexData(vertexData->getTypeName());

        // Create Pipeline(VertexData)
        // Descriptors:
        // ViewData => global, static
        // VertexData => per meshtype
        // SceneData => per meshtype
        Gfx::PermutationId id(permutation);

        Gfx::ORenderCommand command = graphics->createRenderCommand("DepthRender");
        command->setViewport(viewport);

        const Gfx::ShaderCollection *collection = graphics->getShaderCompiler()->findShaders(id);
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
                    .samples = viewport->getSamples(),
                },
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
            Gfx::LegacyPipelineCreateInfo pipelineInfo = {
                .vertexShader = collection->vertexShader,
                .fragmentShader = collection->fragmentShader,
                .renderPass = renderPass,
                .pipelineLayout = collection->pipelineLayout,
                .multisampleState = {
                    .samples = viewport->getSamples(),
                },
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
        command->bindDescriptor(vertexData->getVertexDataSet());
        command->bindDescriptor(viewParamsSet);
        command->bindDescriptor(vertexData->getInstanceDataSet());
        uint32 offset = 0;
        command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexData::DrawCallOffsets), &offset);
        if (graphics->supportMeshShading())
        {
            command->drawMesh(vertexData->getNumInstances(), 1, 1);
        }
        else
        {
            const auto &materials = vertexData->getMaterialData();
            for (const auto &materialData : materials)
            {
                // material not used for any active meshes, skip
                if (materialData.instances.size() == 0)
                    continue;
                for (const auto &drawCall : materialData.instances)
                {
                    command->bindIndexBuffer(vertexData->getIndexBuffer());
                    uint32 inst = drawCall.offsets.instanceOffset;
                    for (const auto &meshData : drawCall.instanceMeshData)
                    {
                        // all meshlets of a mesh share the same indices offset
                        command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex, vertexData->getIndicesOffset(meshData.meshletOffset), inst++);
                    }
                }
            }
        }
        commands.add(std::move(command));
    }

    graphics->executeCommands(std::move(commands));
    graphics->endRenderPass();
    // Sync depth read/write with depth pass depth read
    depthBuffer->pipelineBarrier(
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
    // sync visibility write with depth pass visibility write
    visibilityBuffer->pipelineBarrier(
        Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

void CachedDepthPass::endFrame()
{
}

void CachedDepthPass::publishOutputs()
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
                                    Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("DEPTHPREPASS_DEPTH", depthAttachment);

    TextureCreateInfo visibilityInfo = {
        .format = Gfx::SE_FORMAT_R32_UINT,
        .width = viewport->getOwner()->getFramebufferWidth(),
        .height = viewport->getOwner()->getFramebufferHeight(),
        .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    };
    visibilityBuffer = graphics->createTexture2D(visibilityInfo);
    visibilityAttachment =
        Gfx::RenderTargetAttachment(visibilityBuffer,
                                    Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("VISIBILITY", visibilityAttachment);
}

void CachedDepthPass::createRenderPass()
{
    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = {visibilityAttachment},
        .depthAttachment = depthAttachment,
    };
    Array<Gfx::SubPassDependency> dependency = {
        //    {
        //        .srcSubpass = 0,
        //        .dstSubpass = ~0U,
        //        .srcStage = Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        //        .dstStage = Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        //        .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        //        .dstAccess = Gfx::SE_ACCESS_SHADER_READ_BIT,
        //    },
        //    {
        //        .srcSubpass = 0,
        //        .dstSubpass = ~0U,
        //        .srcStage = Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        //        .dstStage = Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        //        .srcAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        //        .dstAccess = Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        //    },
        //    {
        //        .srcSubpass = 0,
        //        .dstSubpass = ~0U,
        //        .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        //        .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        //        .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        //        .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        //    }
    };
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), viewport);
}
