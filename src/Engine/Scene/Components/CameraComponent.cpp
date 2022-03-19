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
    rotationX = 0;
    rotationY = 0;
    setRelativeLocation(Vector(0, 10, -50));
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::mouseMove(float deltaX, float deltaY) 
{
    rotationX -= deltaX / 1000.f;
    rotationY += deltaY / 1000.f;
    //std::cout << "X:" << rotationX << " Y: " << rotationY << std::endl;
    setRelativeRotation(Vector(rotationY, rotationX, 0));
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
    Vector eyePos = getTransform().getPosition();
    Vector lookAt = eyePos + getTransform().getForward();
    std::cout << "Eye: " << eyePos << " lookAt: " << lookAt << std::endl;
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


