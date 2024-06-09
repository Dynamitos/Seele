#pragma once
#include "Component/Camera.h"
#include "ComponentSystem.h"

namespace Seele {
namespace System {
class CameraUpdater : public ComponentSystem<Component::Camera> {
  public:
    CameraUpdater(PScene scene);
    virtual ~CameraUpdater();

    virtual void update(Component::Camera& camera);

  private:
};
} // namespace System
} // namespace Seele