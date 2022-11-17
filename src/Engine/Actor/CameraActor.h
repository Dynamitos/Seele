#pragma once
#include "Actor.h"
#include "Component/Camera.h"

namespace Seele
{
class CameraActor : public Actor
{
public:
    CameraActor(PScene scene);
    virtual ~CameraActor();
    Component::Camera& getCameraComponent();
    const Component::Camera& getCameraComponent() const;
private:
};
DEFINE_REF(CameraActor)
} // namespace Seele
