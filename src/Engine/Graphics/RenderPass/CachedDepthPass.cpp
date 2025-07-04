#include "CachedDepthPass.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Shader.h"

using namespace Seele;

CachedDepthPass::CachedDepthPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics), scene(scene) {
    depthPrepassLayout = graphics->createPipelineLayout("CachedDepthLayout");
    depthPrepassLayout->addDescriptorLayout(viewParamsLayout);
    depthPrepassLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(VertexData::DrawCallOffsets),
    });
    if (graphics->supportMeshShading()) {
        graphics->getShaderCompiler()->registerRenderPass("CachedDepthPass", Gfx::PassConfig{
                                                                                 .baseLayout = depthPrepassLayout,
                                                                                 .taskFile = "DrawListTask",
                                                                                 .mainFile = "DrawListMesh",
                                                                                 .fragmentFile = "VisibilityPass",
                                                                                 .hasFragmentShader = true,
                                                                                 .useMeshShading = true,
                                                                                 .hasTaskShader = true,
                                                                                 .useMaterial = false,
                                                                                 .useVisibility = true,
                                                                             });
    } else {
        graphics->getShaderCompiler()->registerRenderPass("CachedDepthPass", Gfx::PassConfig{
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

CachedDepthPass::~CachedDepthPass() {}

void CachedDepthPass::beginFrame(const Component::Camera& cam, const Component::Transform& transform) {
    viewParamsSet = createViewParamsSet(cam, transform);
}

void CachedDepthPass::render() {
    graphics->beginDebugRegion("CachedDepth");
    query->beginQuery();
    timestamps->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "CachedBegin");
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;

    Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("CachedDepthPass");
    permutation.setPositionOnly(getGlobals().usePositionOnly);
    permutation.setDepthCulling(true);
    for (VertexData* vertexData : VertexData::getList()) {
        permutation.setVertexData(vertexData->getTypeName());
        vertexData->getInstanceDataSet()->updateBuffer(VertexData::CULLINGDATA_NAME, 0, cullingBuffer);
        vertexData->getInstanceDataSet()->writeChanges();

        // Create Pipeline(VertexData)
        // Descriptors:
        // ViewData => global, static
        // VertexData => per meshtype
        // SceneData => per meshtype
        Gfx::PermutationId id(permutation);

        Gfx::ORenderCommand command = graphics->createRenderCommand("CullingRender");
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
                .rasterizationState =
                    {
                        .cullMode = Gfx::SE_CULL_MODE_BACK_BIT,
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
                .rasterizationState =
                    {
                        .cullMode = Gfx::SE_CULL_MODE_BACK_BIT,
                    },
                .colorBlend =
                    {
                        .attachmentCount = 1,
                    },
            };
            Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
            command->bindPipeline(pipeline);
        }
        command->bindDescriptor({viewParamsSet, vertexData->getVertexDataSet(), vertexData->getInstanceDataSet()});
        VertexData::DrawCallOffsets offsets = {
            .instanceOffset = 0,
        };
        command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexData::DrawCallOffsets),
                               &offsets);
        if (graphics->supportMeshShading()) {
            command->drawMesh((uint32)vertexData->getNumInstances(), 1, 1);
        } else {
            const auto& materials = vertexData->getMaterialData();
            for (const auto& materialData : materials) {
                // material not used for any active meshes, skip
                if (materialData.instances.size() == 0)
                    continue;
                for (const auto& drawCall : materialData.instances) {
                    command->bindIndexBuffer(vertexData->getIndexBuffer());
                    uint32 inst = drawCall.offsets.instanceOffset;
                    for (const auto& meshData : drawCall.instanceMeshData) {
                        // all meshlets of a mesh share the same indices offset
                        command->drawIndexed(meshData.indicesRange.size, 1, meshData.indicesRange.offset,
                                             vertexData->getIndicesOffset(meshData.meshletRange.offset), inst++);
                    }
                }
            }
        }
        commands.add(std::move(command));
    }

    graphics->executeCommands(std::move(commands));
    graphics->endRenderPass();
    timestamps->write(Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, "CachedEnd");
    query->endQuery();
    graphics->endDebugRegion();
}

void CachedDepthPass::endFrame() {}

void CachedDepthPass::publishOutputs() {
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
        Gfx::RenderTargetAttachment(depthBuffer->getDefaultView(), Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
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
        Gfx::RenderTargetAttachment(visibilityBuffer->getDefaultView(), Gfx::SE_IMAGE_LAYOUT_UNDEFINED, Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    Gfx::SE_ATTACHMENT_LOAD_OP_CLEAR, Gfx::SE_ATTACHMENT_STORE_OP_STORE);
    resources->registerRenderPassOutput("VISIBILITY", visibilityAttachment);
    query = graphics->createPipelineStatisticsQuery("CachedPipelineStatistics");
    resources->registerQueryOutput("CACHED_QUERY", query);

    timestamps = graphics->createTimestampQuery(7, "CachedTS");
    resources->registerTimestampQueryOutput("TIMESTAMPS", timestamps);
}

void CachedDepthPass::createRenderPass() {
    cullingBuffer = resources->requestBuffer("CULLINGBUFFER");

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
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), viewport->getRenderArea(), "CachedDepthPass");
}
