#pragma once
#include "Component.h"
#include "Math/Matrix.h"
#include "Graphics/GraphicsResources.h"
#include "Transform.h"

namespace Seele
{
namespace Component
{
struct Camera
{
    REQUIRE_COMPONENT(Transform)
    
    Camera();
    ~Camera();

    Math::Matrix4 getViewMatrix() const
    {
        assert (!bNeedsViewBuild);
        return viewMatrix;
    }
    Math::Matrix4 getProjectionMatrix() const
    {
        assert (!bNeedsProjectionBuild);
        return projectionMatrix;
    }
    Math::Vector getCameraPosition() const
    {
        return Math::Vector(viewMatrix[3]);
    }
    void setViewport(Gfx::PViewport viewport);
    void mouseMove(float deltaX, float deltaY);
    void mouseScroll(float x);
    void moveX(float amount);
    void moveY(float amount);
    float aspectRatio;
    float fieldOfView;
    void buildViewMatrix();
    void buildProjectionMatrix();
    Math::Matrix4 viewMatrix;
    Math::Matrix4 projectionMatrix;
private:
    bool bNeedsViewBuild;
    bool bNeedsProjectionBuild;

    Gfx::PViewport viewport;

    //Transforms relative to actor
    float yaw;
    float pitch;
};
} // namespace Component
} // namespace Seele
