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
    Math::Vector getCameraPosition() const
    {
        return Math::Vector(viewMatrix[3]);
    }
    void setViewport(Gfx::PViewport viewport);
    void mouseMove(float deltaX, float deltaY);
    void mouseScroll(float x);
    void moveX(float amount);
    void moveY(float amount);
    void buildViewMatrix();
    Math::Matrix4 viewMatrix;
    //Transforms relative to actor
    float yaw;
    float pitch;
private:
    bool bNeedsViewBuild;
};
} // namespace Component
} // namespace Seele
