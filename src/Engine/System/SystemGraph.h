#pragma once
#include "Containers/Array.h"
#include "SystemBase.h"

namespace Seele
{
class SystemGraph
{
public:
    void addSystem(System::UPSystemBase system);
    void run(dp::thread_pool<>& threadPool, float deltaTime);
private:
    Array<System::UPSystemBase> systems;
};
DEFINE_REF(SystemGraph)
} // namespace Seele;