#include "RenderPass.h"

using namespace Seele;

RenderPass::RenderPass(Gfx::PGraphics graphics) : graphics(graphics) {
    viewParamsLayout = graphics->createDescriptorLayout("pViewParams");
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 0,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .uniformLength = sizeof(ViewParameter),
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
    Matrix4 cameraMatrix = cam.getViewMatrix();
    Matrix4 projectionMatrix = viewport->getProjectionMatrix();
    viewParams = {
        .viewMatrix = cameraMatrix,
        .inverseViewMatrix = glm::inverse(cameraMatrix),
        .projectionMatrix = projectionMatrix,
        .inverseProjection = glm::inverse(projectionMatrix),
        .viewProjectionMatrix = projectionMatrix * cameraMatrix,
        .inverseViewProjectionMatrix = glm::inverse(projectionMatrix * cameraMatrix),
        .cameraPosition_WS = Vector4(cam.getCameraPosition(), 1),
        .cameraForward_WS = Vector4(cam.getCameraForward(), 1),
        .screenDimensions = screenDim,
        .invScreenDimensions = 1.0f / screenDim,
        .frameIndex = Gfx::getCurrentFrameIndex(),
        .time = static_cast<float>(Gfx::getCurrentFrameTime()),
    };

    // screen space
    StaticArray<Vector4, 4> corners = {
        Vector4(0, 0, -1, 1),
        Vector4(screenDim.x, 0, -1, 1),
        Vector4(screenDim.y, 0, -1, 1),
        Vector4(screenDim.x, screenDim.y, -1, 1),
    };

    for (uint32 i = 0; i < corners.size(); ++i) {
        Vector2 texCoord = Vector2(corners[i].x, corners[i].y) / screenDim;

        Vector4 clip = Vector4(Vector2(texCoord.x, 1 - texCoord.y) * 2.0f - 1.0f, corners[i].z, corners[i].w);

        Vector4 world = viewParams.inverseViewProjectionMatrix * clip;

        corners[i] = world / world.w;
    }

    //extract_planes_from_view_projection_matrix(viewParams.viewProjectionMatrix, viewParams.viewFrustum);

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

void RenderPass::normalize_plane(Plane& plane) {
    float l = sqrtf(plane.n.x * plane.n.x + plane.n.y * plane.n.y + plane.n.z * plane.n.z);
    plane.n.x /= l;
    plane.n.y /= l;
    plane.n.z /= l;
    plane.d /= l;
}

void RenderPass::extract_planes_from_view_projection_matrix(const Matrix4 viewProj, Frustum& frustum) {
    // Compute all the planes
    frustum.planes[0] = {
        .n = Vector(viewProj[0][0] + viewProj[0][3], viewProj[1][0] + viewProj[1][3], viewProj[2][0] + viewProj[2][3]),
        .d = viewProj[3][0] + viewProj[3][3],
    };
    frustum.planes[1] = {
        .n = Vector(-viewProj[0][0] + viewProj[0][3], -viewProj[1][0] + viewProj[1][3], -viewProj[2][0] + viewProj[2][3]),
        .d = -viewProj[3][0] + viewProj[3][3],
    };
    frustum.planes[2] = {
        .n = Vector(viewProj[0][1] + viewProj[0][3], viewProj[1][1] + viewProj[1][3], viewProj[2][1] + viewProj[2][3]),
        .d = viewProj[3][1] + viewProj[3][3],
    };
    frustum.planes[3] = {
        .n = Vector(-viewProj[0][1] + viewProj[0][3], -viewProj[1][1] + viewProj[1][3], -viewProj[2][1] + viewProj[2][3]),
        .d = -viewProj[3][1] + viewProj[3][3],
    };
    // Normalize all the planes
    for (uint32_t planeIdx = 0; planeIdx < 4; ++planeIdx)
        normalize_plane(frustum.planes[planeIdx]);
}
