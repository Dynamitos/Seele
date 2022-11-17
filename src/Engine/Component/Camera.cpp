#include "Camera.h"
#include "Actor/Actor.h"
#include <algorithm>
#include <iostream>

using namespace Seele;
using namespace Seele::Component;
using namespace Seele::Math;

Camera::Camera()
    : aspectRatio(0)
    , fieldOfView(glm::radians(70.f))
    , bNeedsViewBuild(false)
    , bNeedsProjectionBuild(false)
    , viewMatrix(Matrix4())
    , projectionMatrix(Matrix4())
{
    yaw = 0;
    pitch = 0;
}

Camera::~Camera()
{
}

void Camera::mouseMove(float deltaYaw, float deltaPitch) 
{
    yaw -= deltaYaw / 500.f;
    pitch += deltaPitch / 500.f;
    //std::cout << "Yaw: " << yaw << " Pitch: " << pitch << std::endl;
    Vector cameraDirection = glm::normalize(Vector(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
    Vector xyz = glm::cross(cameraDirection, Vector(0, 0, 1));
    Quaternion result = Quaternion(glm::dot(cameraDirection, Vector(0, 0, 1)) + 1, xyz.x, xyz.y, xyz.z);
    //std::cout << "Result " << Vector(0, 0, 1) * glm::normalize(result) << " cameraDirection: " << cameraDirection << std::endl;
    getTransform().setRelativeRotation(result);
    bNeedsViewBuild = true;
}

void Camera::mouseScroll(float x) 
{
    getTransform().addRelativeLocation(getTransform().getForward()*x);
    bNeedsViewBuild = true;
}

void Camera::moveX(float amount)
{
    getTransform().addRelativeLocation(getTransform().getForward()*amount);
    bNeedsViewBuild = true;
}

void Camera::moveY(float amount)
{
    getTransform().addRelativeLocation(getTransform().getRight()*amount);
    bNeedsViewBuild = true;
}

void Camera::setViewport(Gfx::PViewport newViewport)
{
    viewport = newViewport;
    aspectRatio = viewport->getSizeX() / (float)viewport->getSizeY();

    bNeedsProjectionBuild = true;
}

void Camera::buildViewMatrix() 
{
    Vector eyePos = getTransform().getPosition();//getAbsoluteTransform().getPosition();
    Vector lookAt = eyePos + glm::normalize(Vector(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
    //std::cout << "Eye: " << eyePos << " lookAt: " << lookAt << std::endl;
    viewMatrix = glm::lookAt(eyePos, lookAt, Vector(0, 1, 0));

    bNeedsViewBuild = false;
}

void Camera::buildProjectionMatrix() 
{
    projectionMatrix = glm::perspective(fieldOfView, aspectRatio, 1.0f, 1000.f);
    static Matrix4 correctionMatrix =
        Matrix4(
            Vector4(1, 0, 0, 0),
            Vector4(0, 1, 0, 0),
            Vector4(0, 0, 0.5f, 0),
            Vector4(0, 0, 0.5f, 1));
    projectionMatrix = correctionMatrix * projectionMatrix;
    bNeedsProjectionBuild = false;
}


