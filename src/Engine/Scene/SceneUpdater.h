#pragma once
#include "Containers/Array.h"

namespace Seele
{
class SceneUpdater
{
public:
    SceneUpdater();
    ~SceneUpdater();
private:
    Array<std::thread> workers;
    List<std::function<void(float)> pendingUpdates;
    void work();
};
} // namespace Seele
