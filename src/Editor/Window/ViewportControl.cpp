#include "ViewportControl.h"
#include "Component/Camera.h"
#include <iostream>

using namespace Seele;
using namespace Seele::Editor;

ViewportControl::ViewportControl(const URect& viewportDimensions, Vector initialPos)
    : position(initialPos)
    , fieldOfView(glm::radians(70.f))
    , aspectRatio(static_cast<float>(viewportDimensions.size.x) / viewportDimensions.size.y)
    , pitch(0)
    , yaw(glm::pi<float>()/-2.0f)
{
    std::cout << yaw << " " << pitch << std::endl;
}

ViewportControl::~ViewportControl()
{
    
}

void ViewportControl::update(Component::Camera& camera, float deltaTime)
{
    float cameraMove = deltaTime * 20;
    if(keys[KeyCode::KEY_LEFT_SHIFT])
    {
        cameraMove *= 4;
    }
    Vector moveVector = Vector();
    Vector forward = glm::normalize(springArm);
    Vector side = glm::cross(Vector(0, 1, 0), forward);
    if(keys[KeyCode::KEY_W])
    {
        moveVector += forward * cameraMove;
    }
    if(keys[KeyCode::KEY_S])
    {
        moveVector += forward * -cameraMove;
    }
    if(keys[KeyCode::KEY_A])
    {
        moveVector += side * cameraMove;
    }
    if(keys[KeyCode::KEY_D])
    {
        moveVector += side * -cameraMove;
    }
    if(keys[KeyCode::KEY_E])
    {
        moveVector += glm::vec3(0, cameraMove, 0);
    }      
    if(keys[KeyCode::KEY_Q])
    {
        moveVector += glm::vec3(0, -cameraMove, 0);
    }
    throw std::logic_error("Not implemented");
}

void ViewportControl::keyCallback(KeyCode key, InputAction action)
{
    keys[static_cast<size_t>(key)] = action != InputAction::RELEASE;
}

void ViewportControl::mouseMoveCallback(double xPos, double yPos)
{
    mouseX = static_cast<float>(xPos);
    mouseY = static_cast<float>(yPos);
}

void ViewportControl::mouseButtonCallback(MouseButton button, InputAction action)
{
    if(button == MouseButton::MOUSE_BUTTON_1)
    {
        mouse1 = action != InputAction::RELEASE;
    }
    if(button == MouseButton::MOUSE_BUTTON_2)
    {
        mouse2 = action != InputAction::RELEASE;
    }
}

void ViewportControl::viewportResize(URect dimensions)
{
    aspectRatio = static_cast<float>(dimensions.size.x) / dimensions.size.y;
}
