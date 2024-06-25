#include "DepthCullingPass.h"
#include "Graphics/Shader.h"
#include <minmax.h>

using namespace Seele;

extern bool usePositionOnly;
extern bool useDepthCulling;

DepthCullingPass::DepthCullingPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics, scene) {
    depthAttachmentLayout = graphics->createDescriptorLayout("pDepthAttachment");
    depthAttachmentLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .shaderStages = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_MESH_BIT_EXT | Gfx::SE_SHADER_STAGE_COMPUTE_BIT,
    });
    depthAttachmentLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .shaderStages = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_COMPUTE_BIT,
    });
    depthAttachmentLayout->create();

    depthCullingLayout = graphics->createPipelineLayout("DepthPrepassLayout");
    depthCullingLayout->addDescriptorLayout(viewParamsLayout);
    depthCullingLayout->addDescriptorLayout(depthAttachmentLayout);
    depthCullingLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(VertexData::DrawCallOffsets),
    });

    depthComputeLayout = graphics->createPipelineLayout("DepthComputeLayout");
    depthComputeLayout->addDescriptorLayout(viewParamsLayout);
    depthComputeLayout->addDescriptorLayout(depthAttachmentLayout);
    depthComputeLayout->addPushConstants(Gfx::SePushConstantRange{
        .stageFlags = Gfx::SE_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = sizeof(MipParam),
    });

    if (graphics->supportMeshShading()) {
        graphics->getShaderCompiler()->registerRenderPass("DepthPass", Gfx::PassConfig{
                                                                           .baseLayout = depthCullingLayout,
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
                                                                           .baseLayout = depthCullingLayout,
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

DepthCullingPass::~DepthCullingPass() {}

void DepthCullingPass::beginFrame(const Component::Camera& cam) { RenderPass::beginFrame(cam); }

void DepthCullingPass::render() {
    depthAttachment.getTexture()->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                               Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, Gfx::SE_ACCESS_TRANSFER_READ_BIT,
                                               Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);

    Gfx::PDescriptorSet set = depthAttachmentLayout->allocateDescriptorSet();
    set->updateTexture(0, Gfx::PTexture2D(depthAttachment.getTexture()));
    set->updateBuffer(1, depthMipBuffer);
    set->writeChanges();

    Gfx::OComputeCommand computeCommand = graphics->createComputeCommand("DepthMipGenCommand");
    computeCommand->bindPipeline(depthInitialReduce);
    computeCommand->bindDescriptor({viewParamsSet, set});
    UVector2 reduceDimensions = UVector2(viewport->getOwner()->getFramebufferWidth() + BLOCK_SIZE - 1,
                                         viewport->getOwner()->getFramebufferHeight() + BLOCK_SIZE - 1) /
                                uint32(BLOCK_SIZE);
    computeCommand->dispatch(reduceDimensions.x, reduceDimensions.y, 1);

    computeCommand->bindPipeline(depthMipGen);
    computeCommand->bindDescriptor({viewParamsSet, set});

    for (uint32 i = 0; i < mipOffsets.size() - 1; ++i) {
        depthMipBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                        Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                        Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        MipParam params = {
            .srcMipOffset = mipOffsets[i],
            .dstMipOffset = mipOffsets[i + 1],
            .srcMipDim = mipDims[i],
            .dstMipDim = mipDims[i + 1],
        };
        computeCommand->pushConstants(Gfx::SE_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(MipParam), &params);
        UVector2 threadGroups = (((mipDims[i] + UVector2(1, 1)) / 2u) + UVector2(BLOCK_SIZE - 1, BLOCK_SIZE - 1)) / uint32(BLOCK_SIZE);
        computeCommand->dispatch(threadGroups.x, threadGroups.y, 1);
    }
    Array<Gfx::OComputeCommand> computeCommands;
    computeCommands.add(std::move(computeCommand));
    graphics->executeCommands(std::move(computeCommands));
    depthMipBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                    Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT);

    depthAttachment.getTexture()->changeLayout(
        Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, Gfx::SE_ACCESS_TRANSFER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

    query->beginQuery();
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;

    Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("DepthPass");
    permutation.setPositionOnly(usePositionOnly);
    permutation.setDepthCulling(useDepthCulling);
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
                .colorBlend =
                    {
                        .attachmentCount = 1,
                    },
            };
            Gfx::PGraphicsPipeline pipeline = graphics->createGraphicsPipeline(std::move(pipelineInfo));
            command->bindPipeline(pipeline);
        }
        command->bindDescriptor({viewParamsSet, vertexData->getVertexDataSet(), vertexData->getInstanceDataSet(), set});
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
    query->endQuery();
    // Sync depth read/write with compute read
    depthAttachment.getTexture()->pipelineBarrier(
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    // Sync visibility write with compute read
    visibilityAttachment.getTexture()->pipelineBarrier(Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                       Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                                       Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void DepthCullingPass::endFrame() {}

void DepthCullingPass::publishOutputs() {
    uint32 width = viewport->getOwner()->getFramebufferWidth();
    uint32 height = viewport->getOwner()->getFramebufferHeight();
    uint32 bufferSize = 0;
    while (width > 1 && height > 1) {
        mipOffsets.add(bufferSize);
        mipDims.add(UVector2(width, height));
        bufferSize += width * height;
        width = max((width + 1) / 2, 1);
        height = max((height + 1) / 2, 1);
    }
    ShaderBufferCreateInfo depthMipInfo = {
        .sourceData =
            {
                .size = bufferSize * sizeof(uint32),
                .data = nullptr,
            },
        .numElements = bufferSize,
        .name = "DepthMipBuffer",
    };
    depthMipBuffer = graphics->createShaderBuffer(depthMipInfo);

    ShaderCreateInfo mipComputeInfo = {
        .name = "DepthMipCompute",
        .mainModule = "DepthMipGen",
        .entryPoint = "initialReduce",
        .rootSignature = depthComputeLayout,
    };

    depthInitialReduceShader = graphics->createComputeShader(mipComputeInfo);
    depthComputeLayout->create();

    Gfx::ComputePipelineCreateInfo pipelineCreateInfo = {
        .computeShader = depthInitialReduceShader,
        .pipelineLayout = depthComputeLayout,
    };
    depthInitialReduce = graphics->createComputePipeline(pipelineCreateInfo);

    mipComputeInfo.entryPoint = "reduceLevel";

    depthMipGenShader = graphics->createComputeShader(mipComputeInfo);

    pipelineCreateInfo.computeShader = depthMipGenShader;

    depthMipGen = graphics->createComputePipeline(pipelineCreateInfo);

    query = graphics->createPipelineStatisticsQuery();
    resources->registerQueryOutput("DEPTH_QUERY", query);
}

void DepthCullingPass::createRenderPass() {
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
