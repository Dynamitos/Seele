#pragma once
#include <thread_pool/thread_pool.h>
#include <entt/entt.hpp>
#include "Scene/Scene.h"

namespace Seele
{
namespace System
{
class SystemBase
{
public:
    SystemBase(PScene scene) : registry(scene->registry), scene(scene) {}
    virtual ~SystemBase() {}
    virtual void run(dp::thread_pool<>& pool, double delta)
    {
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