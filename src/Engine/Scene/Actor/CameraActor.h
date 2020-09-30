#pragma once
#include "Actor.h"

namespace Seele
{
DECLARE_REF(CameraComponent)
class CameraActor : public Actor
{
public:
    CameraActor();
    virtual ~CameraActor();
    PCameraComponent getCameraComponent() const
    {
        return cameraComponent;
    }
private:
    PCameraComponent cameraComponent;
    PComponent sceneComponent; // This will be the root, camera will be the child
};
DEFINE_REF(CameraActor);
} // namespace Seele
