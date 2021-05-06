#include "SceneUpdater.h"
#include "Components/Component.h"
#include "Actor/Actor.h"

using namespace Seele;

SceneUpdater::SceneUpdater()
    : pendingUpdatesSem(0)
{
    running.store(true);
    workers.resize(std::thread::hardware_concurrency());
    for (size_t i = 0; i < workers.size(); i++)
    {
        workers[i] = std::thread(&SceneUpdater::work, this);
    }    
}

SceneUpdater::~SceneUpdater() 
{
    running.store(false);
    for (size_t i = 0; i < workers.size(); i++)
    {
        workers[i].join();
    }
}

void SceneUpdater::registerComponentUpdate(PComponent component) 
{
    std::unique_lock lck(pendingUpdatesLock);
    updatesRan.add([component](float delta) mutable { component->tick(delta); });
}

void SceneUpdater::registerActorUpdate(PActor actor) 
{
    std::unique_lock lck(pendingUpdatesLock);
    updatesRan.add([actor](float delta) mutable { actor->tick(delta); });
}

void SceneUpdater::runUpdates(float delta) 
{
    
    std::unique_lock pendingLock(pendingUpdatesLock);
    frameDelta = delta;
    pendingUpdates = std::move(updatesRan);
    updatesRan = List<std::function<void(float)>>();
    
    std::unique_lock lck(frameFinishedLock);
    pendingLock.unlock();
    pendingUpdatesSem.release(pendingUpdates.size());
    frameFinishedCV.wait(lck);
}

void SceneUpdater::work() 
{
    while(running.load())
    {
        pendingUpdatesSem.acquire();
        std::function<void(float)> function;
        {
            std::unique_lock lck(pendingUpdatesLock);
            function = std::move(pendingUpdates.front());
            pendingUpdates.popFront();
            if(pendingUpdates.empty())
            {
                std::unique_lock lck(frameFinishedLock);
                frameFinishedCV.notify_all();
            }
        }
        function(frameDelta);
        {
            std::unique_lock lck(pendingUpdatesLock);
            updatesRan.add(std::move(function));
        }
    }
}
