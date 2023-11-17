#pragma once
#include "Containers/Array.h"
#include "SystemBase.h"

namespace Seele
{
class SystemGraph
{
public:
    void addSystem(System::OSystemBase system);
    void run(dp::thread_pool<>& threadPool, float deltaTime);
private:
    Array<System::OSystemBase> systems;
};
DEFINE_REF(SystemGraph)
} // namespace Seele;