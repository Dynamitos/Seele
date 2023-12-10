#include "Camera.h"
#include "Actor/Actor.h"
#include <algorithm>
#include <iostream>

using namespace Seele;
using namespace Seele::Component;
using namespace Seele::Math;

Camera::Camera()
    : viewMatrix(Matrix4())
    , bNeedsViewBuild(false)
    , cameraPos(Vector())
{
    yaw = 3.1415f/2;
    pitch = 0;
}

Camera::~Camera()
{
}

void Camera::mouseMove(float deltaYaw, float deltaPitch) 
{
    yaw += deltaYaw / 500.f;
    pitch += deltaPitch / 500.f;
    Vector cameraDirection = glm::normalize(Vector(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
    Matrix4 viewMat = glm::lookAt(getTransform().getPosition(), getTransform().getPosition() + cameraDirection, Vector(0, 1, 0));
    getTransform().setRotation(viewMat); // quaternion from matrix, janky but im bad at math
    bNeedsViewBuild = true;
}

void Camera::mouseScroll(float x) 
{
    getTransform().translate(getTransform().getForward()*x);
    bNeedsViewBuild = true;
}

void Camera::moveX(float amount)
{
    getTransform().translate(getTransform().getForward()*amount);
    bNeedsViewBuild = true;
}

void Camera::moveY(float amount)
{
    getTransform().translate(getTransform().getRight()*amount);
    bNeedsViewBuild = true;
}

void Camera::buildViewMatrix() 
{
    Vector eyePos = getTransform().getPosition();
    Vector lookAt = eyePos + getTransform().getForward();
    viewMatrix = glm::lookAt(eyePos, lookAt, Vector(0, 1, 0));
    cameraPos = eyePos;
    bNeedsViewBuild = false;
}
