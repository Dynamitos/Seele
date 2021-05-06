#pragma once
#include "Containers/List.h"
#include <semaphore>

namespace Seele
{
DECLARE_REF(Component);
DECLARE_REF(Actor);
class SceneUpdater
{
public:
    SceneUpdater();
    ~SceneUpdater();
    void registerComponentUpdate(PComponent component);
    void registerActorUpdate(PActor actor);
    void runUpdates(float delta);
private:
    Array<std::thread> workers;
    std::mutex pendingUpdatesLock;
    std::counting_semaphore<> pendingUpdatesSem;
    List<std::function<void(float)>> pendingUpdates;
    List<std::function<void(float)>> updatesRan;
    void work();
    float frameDelta;
    std::atomic_bool running;
    std::mutex frameFinishedLock;
    std::condition_variable frameFinishedCV;
};
DEFINE_REF(SceneUpdater)
} // namespace Seele
