#include "VisibilityPass.h"

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
    visibilityDescriptor->reset();
    visibilitySet = visibilityDescriptor->allocateDescriptorSet();
    visibilitySet->updateTexture(0, visibilityAttachment->getTexture());
    visibliitySet->updateBuffer(StaticMeshVertexData::getInstance()->get)
}

void VisibilityPass::render()
{

}

void VisibilityPass::endFrame()
{

}

void VisibilityPass::publishOutputs()
{
    visibilityDescriptor = graphcis->createDescriptorLayout("pVisibilityParams");
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
}

void VisibilityPass::createRenderPass()
{
    visibilityAttachment = resources->requestRenderTarget("VISIBILITY");
}
