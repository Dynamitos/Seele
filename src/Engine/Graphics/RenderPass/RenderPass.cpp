#include "RenderPass.h"

using namespace Seele;

RenderPass::RenderPass(Gfx::PGraphics graphics) : graphics(graphics) {
    viewParamsLayout = graphics->createDescriptorLayout("pViewParams");
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "viewMatrix", .uniformLength = sizeof(Matrix4)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "inverseViewMatrix", .uniformLength = sizeof(Matrix4)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "projectionMatrix", .uniformLength = sizeof(Matrix4)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "inverseProjection", .uniformLength = sizeof(Matrix4)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "viewProjectionMatrix", .uniformLength = sizeof(Matrix4)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "inverseViewProjectionMatrix", .uniformLength = sizeof(Matrix4)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "cameraPosition_WS", .uniformLength = sizeof(Vector4)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "cameraForward_WS", .uniformLength = sizeof(Vector4)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "screenDimensions", .uniformLength = sizeof(Vector2)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "invScreenDimensions", .uniformLength = sizeof(Vector2)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "frameIndex", .uniformLength = sizeof(uint32)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "time", .uniformLength = sizeof(float)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "pad0", .uniformLength = sizeof(float)});
    viewParamsLayout->addDescriptorBinding(Gfx::DescriptorBinding{.name = "pad1", .uniformLength = sizeof(float)});
    viewParamsLayout->create();
}

RenderPass::~RenderPass() {}

void RenderPass::setResources(PRenderGraphResources _resources) { resources = _resources; }

void RenderPass::setViewport(Gfx::PViewport _viewport) { viewport = _viewport; }

void RenderPass::updateViewParameters(const Component::Camera& cam, const Component::Transform& transform) {
    auto screenDim = Vector2(static_cast<float>(viewport->getWidth()), static_cast<float>(viewport->getHeight()));
    Vector eyePos = transform.getPosition();
    Vector lookAt = eyePos + transform.getForward();
    Matrix4 cameraMatrix = glm::lookAt(eyePos, lookAt, Vector(0, 1, 0));
    Matrix4 projectionMatrix = viewport->getProjectionMatrix(cam.nearPlane, cam.farPlane);
    viewParams = {
        .viewMatrix = cameraMatrix,
        .inverseViewMatrix = glm::inverse(cameraMatrix),
        .projectionMatrix = projectionMatrix,
        .inverseProjection = glm::inverse(projectionMatrix),
        .viewProjectionMatrix = projectionMatrix * cameraMatrix,
        .inverseViewProjectionMatrix = glm::inverse(projectionMatrix * cameraMatrix),
        .cameraPosition_WS = Vector4(transform.getPosition(), 1),
        .cameraForward_WS = Vector4(transform.getForward(), 1),
        .screenDimensions = screenDim,
        .invScreenDimensions = 1.0f / screenDim,
        .frameIndex = Gfx::getCurrentFrameIndex(),
        .time = static_cast<float>(Gfx::getCurrentFrameTime()),
    };
}

Gfx::PDescriptorSet RenderPass::createViewParamsSet()
{
    // screen space
    //StaticArray<Vector4, 4> corners = {
    //    Vector4(0, 0, -1, 1),
    //    Vector4(screenDim.x, 0, -1, 1),
    //    Vector4(screenDim.y, 0, -1, 1),
    //    Vector4(screenDim.x, screenDim.y, -1, 1),
    //};
    //
    //for (uint32 i = 0; i < corners.size(); ++i) {
    //    Vector2 texCoord = Vector2(corners[i].x, corners[i].y) / screenDim;
    //
    //    Vector4 clip = Vector4(Vector2(texCoord.x, 1 - texCoord.y) * 2.0f - 1.0f, corners[i].z, corners[i].w);
    //
    //    Vector4 world = viewParams.inverseViewProjectionMatrix * clip;
    //
    //    corners[i] = world / world.w;
    //}

    // extract_planes_from_view_projection_matrix(viewParams.viewProjectionMatrix, viewParams.viewFrustum);

    viewParamsLayout->reset();
    Gfx::PDescriptorSet viewParamsSet = viewParamsLayout->allocateDescriptorSet();
    viewParamsSet->updateConstants("viewMatrix", 0, &viewParams.viewMatrix);
    viewParamsSet->updateConstants("inverseViewMatrix", 0, &viewParams.inverseViewMatrix);
    viewParamsSet->updateConstants("projectionMatrix", 0, &viewParams.projectionMatrix);
    viewParamsSet->updateConstants("inverseProjection", 0, &viewParams.inverseProjection);
    viewParamsSet->updateConstants("viewProjectionMatrix", 0, &viewParams.viewProjectionMatrix);
    viewParamsSet->updateConstants("inverseViewProjectionMatrix", 0, &viewParams.inverseViewProjectionMatrix);
    viewParamsSet->updateConstants("cameraPosition_WS", 0, &viewParams.cameraPosition_WS);
    viewParamsSet->updateConstants("cameraForward_WS", 0, &viewParams.cameraForward_WS);
    viewParamsSet->updateConstants("screenDimensions", 0, &viewParams.screenDimensions);
    viewParamsSet->updateConstants("invScreenDimensions", 0, &viewParams.invScreenDimensions);
    viewParamsSet->updateConstants("frameIndex", 0, &viewParams.frameIndex);
    viewParamsSet->updateConstants("time", 0, &viewParams.time);
    viewParamsSet->updateConstants("pad0", 0, &viewParams.pad0);
    viewParamsSet->updateConstants("pad1", 0, &viewParams.pad1);
    viewParamsSet->writeChanges();
    return viewParamsSet;
}

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
