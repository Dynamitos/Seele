#include "CameraComponent.h"
#include "Scene/Actor/Actor.h"
#include <algorithm>
#include <iostream>

using namespace Seele;

CameraComponent::CameraComponent()
    : aspectRatio(0)
    , fieldOfView(0)
    , bNeedsViewBuild(true)
    , bNeedsProjectionBuild(true)
    , projectionMatrix(Matrix4())
    , viewMatrix(Matrix4())
{
    yaw = 0;
    pitch = 0;
    setRelativeLocation(Vector(0, 10, -50));
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::mouseMove(float deltaYaw, float deltaPitch) 
{
    yaw -= deltaYaw / 500.f;
    pitch += deltaPitch / 500.f;
    //std::cout << "Yaw: " << yaw << " Pitch: " << pitch << std::endl;
    Vector cameraDirection = glm::normalize(Vector(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
    Vector xyz = glm::cross(cameraDirection, Vector(0, 0, 1));
    Quaternion result = Quaternion(glm::dot(cameraDirection, Vector(0, 0, 1)) + 1, xyz.x, xyz.y, xyz.z);
    //std::cout << "Result " << Vector(0, 0, 1) * glm::normalize(result) << " cameraDirection: " << cameraDirection << std::endl;
    setRelativeRotation(result);
    bNeedsViewBuild = true;
}

void CameraComponent::mouseScroll(float x) 
{
    addRelativeLocation(getTransform().getForward()*x);
    bNeedsViewBuild = true;
}

void CameraComponent::moveX(float amount)
{
    addRelativeLocation(getTransform().getForward()*amount);
    bNeedsViewBuild = true;
}

void CameraComponent::moveY(float amount)
{
    addRelativeLocation(getTransform().getRight()*amount);
    bNeedsViewBuild = true;
}

void CameraComponent::buildViewMatrix() 
{
    Vector eyePos = getAbsoluteTransform().getPosition();
    Vector lookAt = eyePos + glm::normalize(Vector(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)));
    //std::cout << "Eye: " << eyePos << " lookAt: " << lookAt << std::endl;
    viewMatrix = glm::lookAt(eyePos, lookAt, Vector(0, 1, 0));

    bNeedsViewBuild = false;
}

void CameraComponent::buildProjectionMatrix() 
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


