#include "DepthCullingPass.h"
#include "Graphics/Shader.h"

using namespace Seele;

DepthCullingPass::DepthCullingPass(Gfx::PGraphics graphics, PScene scene) : RenderPass(graphics), scene(scene) {
    depthAttachmentLayout = graphics->createDescriptorLayout("pDepthAttachment");
    depthAttachmentLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = DEPTHTEXTURE_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .shaderStages = Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_MESH_BIT_EXT | Gfx::SE_SHADER_STAGE_COMPUTE_BIT,
        .access = Gfx::SE_DESCRIPTOR_ACCESS_SAMPLE_BIT,
    });
    depthAttachmentLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = DEPTHMIP_NAME,
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

void DepthCullingPass::beginFrame(const Component::Camera& cam, const Component::Transform& transform) { RenderPass::beginFrame(cam, transform); }

void DepthCullingPass::render() {
    query->beginQuery();
    depthAttachment.getTexture()->changeLayout(Gfx::SE_IMAGE_LAYOUT_GENERAL, Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                               Gfx::SE_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                               Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    depthMipBuffer->rotateBuffer(depthMipBuffer->getNumElements() * sizeof(uint32));
    Gfx::PDescriptorSet set = depthAttachmentLayout->allocateDescriptorSet();
    set->updateTexture(DEPTHTEXTURE_NAME, 0, depthAttachment.getTexture());
    set->updateBuffer(DEPTHMIP_NAME, 0, depthMipBuffer);
    set->writeChanges();

    timestamps->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "MipBegin");
    // First we generate a pixel value per thread, while using a whole threadgroup
    // for a single one would be a waste
    Gfx::OComputeCommand computeCommand = graphics->createComputeCommand("MipGen");
    computeCommand->bindPipeline(depthSourceCopy);
    computeCommand->bindDescriptor({viewParamsSet, set});
    computeCommand->dispatch((viewport->getWidth() + BLOCK_SIZE - 1) / BLOCK_SIZE, (viewport->getHeight() + BLOCK_SIZE - 1) / BLOCK_SIZE,
                             1);
    graphics->executeCommands(std::move(computeCommand));
    for (uint32 i = 0; i < mipOffsets.size() - 1; ++i)
    {
        depthMipBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                        Gfx::SE_ACCESS_SHADER_READ_BIT | Gfx::SE_ACCESS_SHADER_WRITE_BIT,
                                        Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        MipParam param = {
            .sourceOffset = mipOffsets[i],
            .destOffset = mipOffsets[i + 1],
            .sourceDim = mipDims[i],
            .destDim = mipDims[i + 1],
        };
        Gfx::OComputeCommand reduceCommand = graphics->createComputeCommand("ReduceCommand");
        reduceCommand->bindPipeline(depthReduceLevel);
        reduceCommand->bindDescriptor({viewParamsSet, set});
        reduceCommand->pushConstants(Gfx::SE_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(MipParam), &param);
        auto dispatchDim = (mipDims[i + 1] + uint32(BLOCK_SIZE) - 1u) / uint32(BLOCK_SIZE);
        reduceCommand->dispatch(dispatchDim.x, dispatchDim.y, 1);
        graphics->executeCommands(std::move(reduceCommand));
    }

    depthMipBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                    Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT);

    depthAttachment.getTexture()->changeLayout(
        Gfx::SE_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, Gfx::SE_ACCESS_TRANSFER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
        Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | Gfx::SE_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        Gfx::SE_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

    timestamps->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "CullingBegin");
    graphics->beginRenderPass(renderPass);
    Array<Gfx::ORenderCommand> commands;

    Gfx::ShaderPermutation permutation = graphics->getShaderCompiler()->getTemplate("DepthPass");
    permutation.setPositionOnly(true);
    permutation.setDepthCulling(getGlobals().useDepthCulling);
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
                .rasterizationState =
                    {
                        .cullMode = Gfx::SE_CULL_MODE_NONE,
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
                        .cullMode = Gfx::SE_CULL_MODE_NONE,
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
        VertexData::DrawCallOffsets offsets = {
            .instanceOffset = 0,
            .textureOffset = 0,
            .samplerOffset = 0,
            .floatOffset = 0,
        };
        command->pushConstants(Gfx::SE_SHADER_STAGE_TASK_BIT_EXT | Gfx::SE_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexData::DrawCallOffsets),
                               &offsets);
        if (graphics->supportMeshShading()) {
            command->drawMesh((uint32)vertexData->getNumInstances(), 1, 1);
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
    timestamps->write(Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, "CullingEnd");
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
        width = (width + 1) / 2;
        height = (height + 1) / 2;
    }

    depthMipBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = bufferSize * sizeof(uint32),
                .data = nullptr,
            },
        .numElements = bufferSize,
        .name = "DepthMipBuffer",
    });

    graphics->beginShaderCompilation(ShaderCompilationInfo{
        .name = "DepthMipCompute",
        .modules = {"DepthMipGen"},
        .entryPoints = {{"sourceCopy", "DepthMipGen"}, {"reduceLevel", "DepthMipGen"}},
        .rootSignature = depthComputeLayout,
    });
    depthSourceCopyShader = graphics->createComputeShader({0});
    depthComputeLayout->create();

    Gfx::ComputePipelineCreateInfo pipelineCreateInfo = {
        .computeShader = depthSourceCopyShader,
        .pipelineLayout = depthComputeLayout,
    };
    depthSourceCopy = graphics->createComputePipeline(pipelineCreateInfo);

    depthReduceLevelShader = graphics->createComputeShader({1});
    pipelineCreateInfo.computeShader = depthReduceLevelShader;
    depthReduceLevel = graphics->createComputePipeline(pipelineCreateInfo);

    query = graphics->createPipelineStatisticsQuery("DepthPipelineStatistics");
    resources->registerQueryOutput("DEPTH_QUERY", query);
}

void DepthCullingPass::createRenderPass() {
    cullingBuffer = resources->requestBuffer("CULLINGBUFFER");
    timestamps = resources->requestTimestampQuery("TIMESTAMPS");

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
    renderPass = graphics->createRenderPass(std::move(layout), std::move(dependency), "DepthCullingPass");
}
