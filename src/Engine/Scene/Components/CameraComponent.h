#pragma once
#include "Component.h"
#include "Math/Matrix.h"

namespace Seele
{
class CameraComponent : public Component
{
public:
    CameraComponent();
    virtual ~CameraComponent();

    Matrix4 getViewMatrix()
    {
        if (bNeedsViewBuild)
        {
            buildViewMatrix();
        }
        return viewMatrix;
    }
    Matrix4 getProjectionMatrix()
    {
        if (bNeedsProjectionBuild)
        {
            buildProjectionMatrix();
        }
        return projectionMatrix;
    }
    Vector getCameraPosition()
    {
        return getTransform().getPosition();
    }
    void mouseMove(float deltaX, float deltaY);
    void mouseScroll(float x);
    void moveX(float amount);
    void moveY(float amount);
    float aspectRatio;
    float fieldOfView;
private:
    bool bNeedsViewBuild;
    bool bNeedsProjectionBuild;
    void buildViewMatrix();
    void buildProjectionMatrix();

    //Transforms relative to actor
    Matrix4 viewMatrix;
    Matrix4 projectionMatrix;
    float yaw;
    float pitch;
};
DEFINE_REF(CameraComponent)
} // namespace Seele
