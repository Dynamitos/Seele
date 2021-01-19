#include "CameraComponent.h"
#include "Scene/Actor/Actor.h"
#include <algorithm>
#include <iostream>

using namespace Seele;

CameraComponent::CameraComponent() 
    : bNeedsProjectionBuild(true)
    , bNeedsViewBuild(true)
    , originPoint(0, 0, 0)
    , cameraPosition(0, 0, 0)
{
    distance = 50;
    rotationX = 0;
    rotationY = -1.f;
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::mouseMove(float deltaX, float deltaY) 
{
    //0.01 mouse sensitivity
	rotationX += deltaX/500.f;
	rotationY += deltaY/500.f;
	rotationY = std::clamp(rotationY, -1.5f, 1.5f);
	getParent()->setWorldRotation(glm::rotate(glm::quat(), rotationX, Vector(0, 1, 0)));
	getParent()->setWorldRotation(glm::rotate(glm::quat(), rotationY, Vector(1, 0, 0)));
	bNeedsViewBuild = true;
}

void CameraComponent::mouseScroll(double x) 
{
    distance -= static_cast<float>(x);
	distance = std::max(distance, 1.f);
	getParent()->addWorldTranslation(Vector(0, 0, x));
	bNeedsViewBuild = true;
}

void CameraComponent::moveOrigin(float up) 
{
	originPoint.y += up;
	getParent()->addWorldTranslation(Vector(0, up, 0));
	bNeedsViewBuild = true;
}

void CameraComponent::buildViewMatrix() 
{
    Matrix4 rotation = glm::rotate(Matrix4(1.0f), rotationX, Vector(0, 1, 0));
	rotation = glm::rotate(rotation, rotationY, Vector(1, 0, 0));
	Vector4 translation(0, 0, distance, 1);
	translation = rotation * translation;

	cameraPosition = originPoint + Vector(translation);
	viewMatrix = glm::lookAt(cameraPosition, originPoint, Vector(0, 1, 0));

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


