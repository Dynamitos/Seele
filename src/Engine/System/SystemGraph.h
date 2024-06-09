#pragma once
#include "Containers/Array.h"
#include "SystemBase.h"
#include "ThreadPool.h"

namespace Seele {
class SystemGraph {
  public:
    void addSystem(System::OSystemBase system);
    void run(float deltaTime);

  private:
    Array<System::OSystemBase> systems;
};
DEFINE_REF(SystemGraph)
} // namespace Seele