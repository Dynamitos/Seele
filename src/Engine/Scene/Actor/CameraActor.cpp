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
    cameraComponent->aspectRatio = 1.777778f;
    cameraComponent->setParent(sceneComponent);
    cameraComponent->setOwner(this);
	addWorldTranslation(Vector(0, 0, 50));
	addWorldRotation(Vector(0, -1, 0));
}

CameraActor::~CameraActor()
{
}