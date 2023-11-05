#pragma once
#include "Component.h"
#include "Math/Matrix.h"
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

    Matrix4 getViewMatrix() const
    {
        assert (!bNeedsViewBuild);
        return viewMatrix;
    }
    Vector getCameraPosition() const
    {
        return Vector(viewMatrix[3]);
    }
    void mouseMove(float deltaX, float deltaY);
    void mouseScroll(float x);
    void moveX(float amount);
    void moveY(float amount);
    void buildViewMatrix();
    Matrix4 viewMatrix;
    //Transforms relative to actor
    float yaw;
    float pitch;
    bool mainCamera = true;
private:
    bool bNeedsViewBuild;
};
} // namespace Component
} // namespace Seele
