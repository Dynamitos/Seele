#pragma once
#include "MinimalEngine.h"
#include "Math/Vector.h"
#include "Component/Camera.h"

namespace Seele
{
class ViewportControl
{
public:
    ViewportControl(const URect& viewportDimensions, Vector initialPos /*TODO: configurable initial rotations*/);
    ~ViewportControl();
    void update(Component::Camera& camera, float deltaTime);
    void keyCallback(KeyCode key, InputAction action);
    void mouseMoveCallback(double xPos, double yPos);
    void mouseButtonCallback(MouseButton button, InputAction action);
    void viewportResize(URect dimensions);
private:
    Vector position;
    Vector springArm;
    float fieldOfView;
    float aspectRatio;
    StaticArray<bool, static_cast<size_t>(KeyCode::KEY_LAST)> keys;
    bool mouse1 = false;
    bool mouse2 = false;
    float mouseX;
    float mouseY;
    float pitch;
    float yaw;
};
} // namespace Seele
