#pragma once
#include "ComponentSystem.h"
#include "Component/Camera.h"

namespace Seele
{
namespace System
{
class CameraUpdater : public ComponentSystem<Component::Camera>
{
public:
    CameraUpdater(PScene scene);
    virtual ~CameraUpdater();

    virtual void update(Component::Camera& camera);
private:
};
}
}