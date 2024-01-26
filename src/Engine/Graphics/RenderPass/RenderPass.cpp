#include "RenderPass.h"

using namespace Seele;

RenderPass::RenderPass(Gfx::PGraphics graphics, PScene scene)
    : graphics(graphics)
    , scene(scene)
{

    viewParamsLayout = graphics->createDescriptorLayout("ViewLayout");
    viewParamsLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    UniformBufferCreateInfo uniformInitializer = {
        .sourceData = {
            .size = sizeof(ViewParameter),
            .data = (uint8*)&viewParams,
        },
        .dynamic = true,
    };
    viewParamsBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewParamsLayout->create();
}

RenderPass::~RenderPass()
{}

void RenderPass::beginFrame(const Component::Camera& cam)
{
    viewParams = {
        .viewMatrix = cam.getViewMatrix(),
        .projectionMatrix = viewport->getProjectionMatrix(),
        .cameraPosition = Vector4(cam.getCameraPosition(), 1),
        .screenDimensions = Vector2(static_cast<float>(viewport->getWidth()), static_cast<float>(viewport->getHeight())),
    };
    DataSource uniformUpdate = {
        .size = sizeof(ViewParameter),
        .data = (uint8*)&viewParams,
    };
    viewParamsBuffer->updateContents(uniformUpdate);
    viewParamsBuffer->pipelineBarrier(
        Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, 
        Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT, 
        Gfx::SE_ACCESS_MEMORY_READ_BIT,
        Gfx::SE_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    viewParamsLayout->reset();
    viewParamsSet = viewParamsLayout->allocateDescriptorSet();
    viewParamsSet->updateBuffer(0, viewParamsBuffer);
    viewParamsSet->writeChanges();
}

void RenderPass::setResources(PRenderGraphResources _resources)
{
    resources = _resources;
}

void RenderPass::setViewport(Gfx::PViewport _viewport)
{
    viewport = _viewport;
    publishOutputs();
}