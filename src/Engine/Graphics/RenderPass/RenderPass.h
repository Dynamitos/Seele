#pragma once
#include "Component/Camera.h"
#include "Graphics/VertexData.h"
#include "Material/MaterialInstance.h"
#include "Math/Math.h"
#include "MinimalEngine.h"
#include "RenderGraphResources.h"
#include "Scene/Scene.h"


namespace Seele {
DECLARE_NAME_REF(Gfx, Viewport)
DECLARE_NAME_REF(Gfx, Graphics)
DECLARE_NAME_REF(Gfx, RenderPass)
class RenderPass {
  public:
    RenderPass(Gfx::PGraphics graphics);
    RenderPass(RenderPass&&) = default;
    RenderPass& operator=(RenderPass&&) = default;
    virtual ~RenderPass();
    virtual void beginFrame(const Component::Camera& cam, const Component::Transform& transform) = 0;
    virtual void render() = 0;
    virtual void endFrame() = 0;
    virtual void publishOutputs() = 0;
    virtual void createRenderPass() = 0;
    void setResources(PRenderGraphResources _resources);
    void setViewport(Gfx::PViewport _viewport);

  protected:
    void updateViewParameters(const Component::Camera& cam, const Component::Transform& transform);
    Gfx::ODescriptorSet createViewParamsSet();
    struct Plane {
        Vector n;
        float d;
    };
    struct Frustum {
        Plane planes[4];
    };
    Plane computePlane(Vector p0, Vector p1, Vector p2) {
        Plane plane;

        Vector v0 = p1 - p0;
        Vector v2 = p2 - p0;

        plane.n = normalize(cross(v0, v2));

        plane.d = dot(plane.n, p0);

        return plane;
    }
    void normalize_plane(Plane& plane);
    void extract_planes_from_view_projection_matrix(const Matrix4 viewProj, Frustum& frustum);
    
    struct ViewParameter {
        Matrix4 viewMatrix;
        Matrix4 inverseViewMatrix;
        Matrix4 projectionMatrix;
        Matrix4 inverseProjection;
        Matrix4 viewProjectionMatrix;
        Matrix4 inverseViewProjectionMatrix;
        Vector4 cameraPosition_WS;
        Vector4 cameraForward_WS;
        Vector2 screenDimensions;
        Vector2 invScreenDimensions;
        uint32 frameIndex;
        float time;
        uint32 pad0;
        uint32 pad1;
    } viewParams;
    PRenderGraphResources resources;
    Gfx::ODescriptorLayout viewParamsLayout;
    Gfx::ORenderPass renderPass;
    Gfx::PGraphics graphics;
    Gfx::PViewport viewport;
};
DEFINE_REF(RenderPass)
template <typename RP>
concept RenderPassType = std::derived_from<RP, RenderPass>;

} // namespace Seele
