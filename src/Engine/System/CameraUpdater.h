#pragma once
#include "Component/Camera.h"
#include "ComponentSystem.h"

namespace Seele {
namespace System {
class CameraUpdater : public ComponentSystem<Component::Camera, Component::Transform> {
  public:
    CameraUpdater(PScene scene);
    virtual ~CameraUpdater();

    virtual void update(Component::Camera& camera, Component::Transform& transform);

  private:
};
} // namespace System
} // namespace Seele