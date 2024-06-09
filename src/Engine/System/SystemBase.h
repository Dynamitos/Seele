#pragma once
#include "Scene/Scene.h"
#include "ThreadPool.h"
#include <entt/entt.hpp>

namespace Seele {
namespace System {
class SystemBase {
  public:
    SystemBase(PScene scene) : registry(scene->registry), scene(scene) {}
    virtual ~SystemBase() {}
    virtual void run(double delta) {
        deltaTime = delta;
        update();
    }
    virtual void update() {}

  protected:
    double deltaTime;
    entt::registry& registry;
    PScene scene;
};
DEFINE_REF(SystemBase)
} // namespace System
} // namespace Seele