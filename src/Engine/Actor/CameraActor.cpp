#include "CameraActor.h"
#include "Scene/Scene.h"

using namespace Seele;

CameraActor::CameraActor(PScene scene)
    : Actor(scene)
{
    attachComponent<Component::Camera>();
    accessComponent<Component::Transform>().setRelativeLocation(Vector(10, 5, 14));
}

CameraActor::~CameraActor()
{
}

Component::Camera& CameraActor::getCameraComponent()
{
    return accessComponent<Component::Camera>();
}

const Component::Camera& CameraActor::getCameraComponent() const
{
    return accessComponent<Component::Camera>();
}