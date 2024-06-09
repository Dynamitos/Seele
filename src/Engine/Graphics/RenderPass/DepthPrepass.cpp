#include "DepthPrepass.h"
#include "Graphics/Shader.h"

using namespace Seele;

extern bool usePositionOnly;
extern bool useViewCulling;

DepthPrepass::DepthPrepass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics, scene) {
    depthPrepassLayout = graphics->createPipelineLayout("DepthPrepassLayout");
    depthPrepassLayout->addDescriptorLayout(viewParamsLayout);
    depthPrepassLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(VertexData::DrawCallOffsets),
    });
    if (graphics->supportMeshShading()) {
        graphics->getShaderCompiler()->registerRenderPass("DepthPass", Gfx::PassConfig{
                                                                           .baseLayout = depthPrepassLayout,
                                                                           .taskFile = "DepthCullingTask",
                                                                           .mainFile = "DepthCullingMesh",
                                                                           .fragmentFile = "VisibilityPass",
                                                                           .hasFragmentShader = true,
                                                                           .useMeshShading = true,
                                                                           .hasTaskShader = true,
                                                                           .useMaterial = false,
                                                                           .useVisibility = true,
                                                                       });
    } else {
        graphics->getShaderCompiler()->registerRenderPass("DepthPass", Gfx::PassConfig{
                                                                           .baseLayout = depthPrepassLayout,
                                                                           .taskFile = "",
                                                                           .mainFile = "LegacyPass",
                                                                           .fragmentFile = "VisibilityPass",
                                                                           .hasFragmentShader = true,
                                                                           .useMeshShading = false,
                                                                           .hasTaskShader = false,
                                                                           .useMaterial = false,
                                                                           .useVisibility = true,
                                                                       });
    }
}

DepthPrepass::~DepthPrepass() {}

void DepthPrepass::beginFrame(const Component::Camera& cam) { RenderPass::beginFrame(cam); }

void DepthPrepass::render() {
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;

    Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("DepthPass");
    permutation.setPositionOnly(usePositionOnly);
    permutation.setViewCulling(useViewCulling);
    for (VertexData* vertexData : VertexData::getList()) {
        permutation.setVertexData(vertexData->getTypeName());
        vertexData->getInstanceDataSet()->updateBuffer(6, cullingBuffer);
        vertexData->getInstanceDataSet()->writeChanges();
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
        if (graphics->supportMeshShading()) {
            Gfx::MeshPipelineCreateInfo pipelineInfo = {
                .taskShader = collection->taskShader,
                .meshShader = collection->meshShader,
                .fragmentShader = collection->fragmentShader,
                .renderPass = renderPass,
                .pipelineLayout = collection->pipelineLayout,
                .multisampleState =
                    {
                        .samples = viewport->getSamples(),
                    },
                .depthStencilState =
                    {
                        .depthCompareOp = Gfx::SE_COMPARE_OP_GREATER,
                    },
                .colorBlend =
                    {
                        .attachmentCount = 1,
                    },
            };
            Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
            command->bindPipeline(pipeline);
        } else {
            Gfx::LegacyPipelineCreateInfo pipelineInfo = {
                .vertexShader = collection->vertexShader,
                .fragmentShader = collection->fragmentShader,
                .renderPass = renderPass,
                .pipelineLayout = collection->pipelineLayout,
                .multisampleState =
                    {
                        .samples = viewport->getSamples(),
                    },
                .depthStencilState =
                    {
                        .depthCompareOp = Gfx::SE_COMPARE_OP_GREATER,
                    },
                .colorBlend =
                    {
                        .attachmentCount = 1,
                    },
            };
            Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
            command->bindPipeline(pipeline);
        }
        command->bindDescriptor(viewParamsSet);
        command->bindDescriptor(vertexData->getVertexDataSet());
        command->bindDescriptor(vertexData->getInstanceDataSet());
        uint32 offset = 0;
        command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexData::DrawCallOffsets),
                               &offset);
        if (graphics->supportMeshShading()) {
            command->drawMesh(vertexData->getNumInstances(), 1, 1);
        } else {
            const auto& materials = vertexData->getMaterialData();
            for (const auto& materialData : materials) {
                for (const auto& drawCall : materialData.instances) {
                    // material not used for any active meshes, skip
                    if (materialData.instances.size() == 0)
                        continue;
                    command->bindIndexBuffer(vertexData->getIndexBuffer());
                    uint32 inst = drawCall.offsets.instanceOffset;
                    for (const auto& meshData : drawCall.instanceMeshData) {
                        // all meshlets of a mesh share the same indices offset
                        command->drawIndexed(meshData.numIndices, 1, meshData.firstIndex,
                                             vertexData->getIndicesOffset(meshData.meshletOffset), inst++);
                    }
                }
            }
        }
        commands.add(std::move(command));
    }

    graphics->executeCommands(std::move(commands));
    graphics->endRenderPass();
    // Sync depth read/write with compute read
    depthAttachment.getTexture()->pipelineBarrier(
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    // Sync depth read/write with base pass read
    // depthAttachment.getTexture()->pipelineBarrier(
    //    Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //    Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //    Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    //    Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
    //);
    // Sync visibility write with compute read
    visibilityAttachment.getTexture()->pipelineBarrier(Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                       Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                                       Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void DepthPrepass::endFrame() {}

void DepthPrepass::publishOutputs() {}

void DepthPrepass::createRenderPass() {
    cullingBuffer = resources->requestBuffer("CULLINGBUFFER");

    depthAttachment = resources->requestRenderTarget("DEPTHPREPASS_DEPTH");
    depthAttachment.setInitialLayout(Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    depthAttachment.setFinalLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL);
    depthAttachment.setLoadOp(Gfx::SE_ATTACHMENT_LOAD_OP_LOAD);
    visibilityAttachment = resources->requestRenderTarget("VISIBILITY");
    visibilityAttachment.setInitialLayout(Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    visibilityAttachment.setFinalLayout(Gfx::SE_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    visibilityAttachment.setLoadOp(Gfx::SE_ATTACHMENT_LOAD_OP_LOAD);

    Gfx::RenderTargetLayout layout = Gfx::RenderTargetLayout{
        .colorAttachments = {visibilityAttachment},
        .depthAttachment = depthAttachment,
    };
    Array<Gfx::SubPassDependency> dependency = {
        {
            .srcSubpass = ~0U,
            .dstSubpass = 0,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
        {
            .srcSubpass = 0,
            .dstSubpass = ~0U,
            .srcStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStage = Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccess = Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
    };
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), viewport);
}
