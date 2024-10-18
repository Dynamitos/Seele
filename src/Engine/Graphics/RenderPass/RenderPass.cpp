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
        .name = "viewParamsBuffer",
    };
    viewParamsBuffer = graphics->createUniformBuffer(uniformInitializer);
    viewParamsLayout->create();
}

RenderPass::~RenderPass() {}

void RenderPass::beginFrame(const Component::Camera& cam) {
    auto screenDim = Vector2(static_cast<float>(viewport->getWidth()), static_cast<float>(viewport->getHeight()));
    viewParams = {
        .viewFrustum =
            {
                .planes =
                    {
                        Plane{
                            .n = Vector(-0.62628, 0, 0.7796),
                            .d = 3.898,
                        },
                        Plane{
                            .n = Vector(0.62628, 0, 0.7796),
                            .d = 3.898,
                        },
                        Plane{
                            .n = Vector(0, 0.81915, 0.57358),
                            .d = 2.86788,
                        },
                        Plane{
                            .n = Vector(0, -0.81915, 0.57358),
                            .d = 2.86788,
                        },
                    },
            },
        .viewMatrix = cam.getViewMatrix(),
        .inverseViewMatrix = glm::inverse(cam.getViewMatrix()),
        .projectionMatrix = viewport->getProjectionMatrix(),
        .inverseProjection = glm::inverse(viewport->getProjectionMatrix()),
        .cameraPosition_WS = Vector4(cam.getCameraPosition(), 1),
        .cameraForward_WS = Vector4(cam.getCameraForward(), 1),
        .screenDimensions = screenDim,
        .invScreenDimensions = 1.0f / screenDim,
        .frameIndex = Gfx::getCurrentFrameIndex(),
        .time = static_cast<float>(Gfx::getCurrentFrameTime()),
    };
    viewParamsBuffer->rotateBuffer(sizeof(ViewParameter));
    viewParamsBuffer->updateContents(0, sizeof(ViewParameter), &viewParams);
    viewParamsBuffer->pipelineBarrier(Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                                      Gfx::SE_ACCESS_UNIFORM_READ_BIT | Gfx::SE_ACCESS_TRANSFER_WRITE_BIT,
                                      Gfx::SE_PIPELINE_STAGE_TASK_SHADER_BIT_EXT | Gfx::SE_PIPELINE_STAGE_MESH_SHADER_BIT_EXT |
                                          Gfx::SE_PIPELINE_STAGE_VERTEX_SHADER_BIT | Gfx::SE_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
                                          Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    viewParamsLayout->reset();
    viewParamsSet = viewParamsLayout->allocateDescriptorSet();
    viewParamsSet->updateBuffer(0, 0, viewParamsBuffer);
    viewParamsSet->writeChanges();
}

void RenderPass::setResources(PRenderGraphResources _resources) { resources = _resources; }

void RenderPass::setViewport(Gfx::PViewport _viewport) { viewport = _viewport; }
