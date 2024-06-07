#include "VisibilityPass.h"
#include "Graphics/Shader.h"

using namespace Seele;

VisibilityPass::VisibilityPass(Gfx::PGraphics graphics, PScene scene)
    : RenderPass(graphics, scene)
{
}

VisibilityPass::~VisibilityPass()
{

}

void VisibilityPass::beginFrame(const Component::Camera& cam)
{
    RenderPass::beginFrame(cam);

    //Array<VertexData::MeshletCullingInfo> cullingData(VertexData::getMeshletCount());
    //std::memset(cullingData.data(), 0xffff, cullingData.size() * sizeof(VertexData::MeshletCullingInfo));
    
    //cullingBuffer->updateContents(ShaderBufferCreateInfo{
    //  .sourceData = {
    //      .size = VertexData::getMeshletCount() * sizeof(VertexData::MeshletCullingInfo),
    //      .data = (uint8 *)cullingData.data(),
    //  },
    //  .numElements = VertexData::getMeshletCount()});
    //cullingBuffer->pipelineBarrier(
    //    Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
    //    Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
    //    Gfx::SE_ACCESS_MEMORY_WRITE_BIT,
    //    Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

}

void VisibilityPass::render()
{
    cullingBuffer->rotateBuffer(VertexData::getMeshletCount() * sizeof(VertexData::MeshletCullingInfo));
    cullingBuffer->clear();

    visibilityDescriptor->reset();
    visibilitySet = visibilityDescriptor->allocateDescriptorSet();
    visibilitySet->updateTexture(0, visibilityAttachment.getTexture());
    visibilitySet->updateBuffer(1, cullingBuffer);
    visibilitySet->writeChanges();
    
    Gfx::OComputeCommand command = graphics->createComputeCommand("VisibilityCommand");
    command->bindPipeline(visibilityPipeline);
    command->bindDescriptor({viewParamsSet, visibilitySet});
    command->dispatch(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
    Array<Gfx::OComputeCommand> commands;
    commands.add(std::move(command));
    graphics->executeCommands(std::move(commands));

    cullingBuffer->pipelineBarrier(
        Gfx::SE_ACCESS_SHADER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        Gfx::SE_ACCESS_SHADER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT);
}

void VisibilityPass::endFrame()
{

}

void VisibilityPass::publishOutputs()
{   
    uint32_t viewportWidth = viewport->getWidth();
    uint32_t viewportHeight = viewport->getHeight();
    threadGroupSize = glm::ceil(glm::vec3(viewportWidth / (float)BLOCK_SIZE, viewportHeight / (float)BLOCK_SIZE, 1));
    visibilityDescriptor = graphics->createDescriptorLayout("pVisibilityParams");
    visibilityDescriptor->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_SAMPLED_IMAGE, });
    visibilityDescriptor->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .access = Gfx::SE_DESCRIPTOR_ACCESS_READ_WRITE_BIT, });
    visibilityDescriptor->create();

    visibilityLayout = graphics->createPipelineLayout("VisibilityLayout");
    visibilityLayout->addDescriptorLayout(viewParamsLayout);
    visibilityLayout->addDescriptorLayout(visibilityDescriptor);
    
    ShaderCreateInfo createInfo = {
        .name = "Visibility",
        .mainModule = "VisibilityCompute",
        .entryPoint = "computeMain",
        .rootSignature = visibilityLayout,
    };
    visibilityShader = graphics->createComputeShader(createInfo);
    visibilityLayout->create();

    Gfx::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.computeShader = visibilityShader;
    pipelineInfo.pipelineLayout = std::move(visibilityLayout);
    visibilityPipeline = graphics->createComputePipeline(std::move(pipelineInfo));

    cullingBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{.dynamic = true, .name = "CullingBuffer"});
    resources->registerBufferOutput("CULLINGBUFFER", cullingBuffer);
}

void VisibilityPass::createRenderPass()
{
    visibilityAttachment = resources->requestRenderTarget("VISIBILITY");
}
