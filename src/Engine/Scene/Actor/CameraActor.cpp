#include "CameraActor.h"
#include "Scene/Components/Component.h"
#include "Scene/Components/CameraComponent.h"

using namespace Seele;

CameraActor::CameraActor()
{
    sceneComponent = new Component();
    setRootComponent(sceneComponent);

    cameraComponent = new CameraComponent();
    cameraComponent->fieldOfView = 70.0f;
    cameraComponent->setParent(sceneComponent);
    cameraComponent->setOwner(this);
    sceneComponent->addChildComponent(cameraComponent);
}

CameraActor::~CameraActor()
{
}