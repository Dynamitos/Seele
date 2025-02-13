#include "VisibilityPass.h"
#include "Graphics/Shader.h"

using namespace Seele;

VisibilityPass::VisibilityPass(Gfx::PGraphics graphics) : RenderPass(graphics) {}

VisibilityPass::~VisibilityPass() {}

void VisibilityPass::beginFrame(const Component::Camera& cam) {
    RenderPass::beginFrame(cam);
    cullingBuffer->rotateBuffer(VertexData::getMeshletCount() * sizeof(VertexData::MeshletCullingInfo), true);
}

void VisibilityPass::render() {
    /*cullingBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_MESH_SHADER_BIT_EXT,
                                   Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    cullingBuffer->clear();

    cullingBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, Gfx::SE_ACCESS_SHADER_READ_BIT,
                                   Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    visibilityDescriptor->reset();
    visibilitySet = visibilityDescriptor->allocateDescriptorSet();
    visibilitySet->updateTexture(0, 0, visibilityAttachment.getTexture());
    visibilitySet->updateBuffer(1, 0, cullingBuffer);
    visibilitySet->writeChanges();

    query->beginQuery();
    timestamps->write(Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT, "VisibilityBegin");
    Gfx::OComputeCommand command = graphics->createComputeCommand("VisibilityCommand");
    command->bindPipeline(visibilityPipeline);
    command->bindDescriptor({viewParamsSet, visibilitySet});
    command->dispatch(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
    Array<Gfx::OComputeCommand> commands;
    commands.add(std::move(command));
    graphics->executeCommands(std::move(commands));
    timestamps->write(Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, "VisibilityEnd");
    query->endQuery();
    cullingBuffer->pipelineBarrier(Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                   Gfx::SE_ACCESS_SHADER_READ_BIT,
                                   Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT | Gfx::SE_PIPELINE_STAGE_MESH_SHADER_BIT_EXT);*/
}

void VisibilityPass::endFrame() {}

void VisibilityPass::publishOutputs() {
    uint32_t viewportWidth = viewport->getWidth();
    uint32_t viewportHeight = viewport->getHeight();
    threadGroupSize = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
    visibilityDescriptor = graphics->createDescriptorLayout("pVisibilityParams");
    visibilityDescriptor->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    });
    visibilityDescriptor->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 1,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_WRITE_BIT,
    });
    visibilityDescriptor->create();

    visibilityLayout = graphics->createPipelineLayout("VisibilityLayout");
    visibilityLayout->addDescriptorLayout(visibilityDescriptor);
    visibilityLayout->addDescriptorLayout(viewParamsLayout);

    ShaderCompilationInfo createInfo = {
        .name = "Visibility",
        .modules = {"VisibilityCompute"},
        .entryPoints = {{"computeMain", "VisibilityCompute"}},
        .rootSignature = visibilityLayout,
    };
    graphics->beginShaderCompilation(createInfo);
    visibilityShader = graphics->createComputeShader({0});
    visibilityLayout->create();

    Gfx::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = visibilityShader;
    pipelineInfo.pipelineLayout = visibilityLayout;
    visibilityPipeline = graphics->createComputePipeline(std::move(pipelineInfo));

    cullingBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .clearValue = 0xffffffff,
        .name = "CullingBuffer",
    });
    resources->registerBufferOutput("CULLINGBUFFER", cullingBuffer);

    query = graphics->createPipelineStatisticsQuery("VisibilityPipelineStatistics");
    resources->registerQueryOutput("VISIBILITY_QUERY", query);
}

void VisibilityPass::createRenderPass() {
    visibilityAttachment = resources->requestRenderTarget("VISIBILITY");
    timestamps = resources->requestTimestampQuery("TIMESTAMPS");
}
