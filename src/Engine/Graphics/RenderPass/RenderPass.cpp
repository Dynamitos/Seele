#include "RenderPass.h"

using namespace Seele;

RenderPass::RenderPass(Gfx::PGraphics graphics, PScene scene) : graphics(graphics), scene(scene) {

    viewParamsLayout = graphics->createDescriptorLayout("pViewParams");
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    });
    UniformBufferCreateInfo uniformInitializer = {
        .sourceData =
            {
                .size = sizeof(ViewParameter),
                .data = (uint8*)&viewParams,
            },
        .dynamic = true,
        .name = "viewParamsBuffer",
    };
    viewParamsBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewParamsLayout->create();
}

RenderPass::~RenderPass() {}

void RenderPass::beginFrame(const Component::Camera& cam) {
    viewParams = {
        .viewMatrix = cam.getViewMatrix(),
        .inverseViewMatrix = glm::inverse(cam.getViewMatrix()),
        .projectionMatrix = viewport->getProjectionMatrix(),
        .inverseProjection = glm::inverse(viewport->getProjectionMatrix()),
        .cameraPosition = Vector4(cam.getCameraPosition(), 1),
        .screenDimensions = Vector2(static_cast<float>(viewport->getWidth()), static_cast<float>(viewport->getHeight())),
    };
    DataSource uniformUpdate = {
        .size = sizeof(ViewParameter),
        .data = (uint8*)&viewParams,
    };
    viewParamsBuffer->rotateBuffer(sizeof(ViewParameter));
    viewParamsBuffer->updateContents(uniformUpdate);
    viewParamsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                      Gfx::SE_ACCESS_UNIFORM_READ_BIT,
                                      Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT | Gfx::SE_PIPELINE_STAGE_MESH_SHADER_BIT_EXT |
                                          Gfx::SE_PIPELINE_STAGE_VERTEX_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    viewParamsLayout->reset();
    viewParamsSet = viewParamsLayout->allocateDescriptorSet();
    viewParamsSet->updateBuffer(0, viewParamsBuffer);
    viewParamsSet->writeChanges();
}

void RenderPass::setResources(PRenderGraphResources _resources) { resources = _resources; }

void RenderPass::setViewport(Gfx::PViewport _viewport) { viewport = _viewport; }
