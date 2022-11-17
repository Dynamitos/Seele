#include "CameraActor.h"
#include "Scene/Scene.h"

using namespace Seele;

CameraActor::CameraActor(PScene scene)
    : Actor(scene)
{
    scene->attachComponent<Component::Camera>(identifier);
    scene->accessComponent<Component::Transform>(identifier).setRelativeLocation(Math::Vector(10, 5, 14));
}

CameraActor::~CameraActor()
{
}

Component::Camera& CameraActor::getCameraComponent()
{
    return scene->accessComponent<Component::Camera>(identifier);
}

const Component::Camera& CameraActor::getCameraComponent() const
{
    return scene->accessComponent<Component::Camera>(identifier);
}