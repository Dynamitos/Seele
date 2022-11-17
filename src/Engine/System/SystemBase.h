#pragma once
#include <entt/entt.hpp>
#include <thread_pool/thread_pool.h>
#include "Component/Component.h"

namespace Seele
{
namespace System
{

template<typename... Components>
class SystemBase 
{
public:
    SystemBase(entt::registry& registry) : registry(registry) {}
    virtual ~SystemBase() {}
    template<typename Comp>
    auto getDependencies()
    {
        return Comp::dependencies;
    }
    template<typename... Dep>
    auto mergeDependencies(Dep... deps)
    {
        return (deps | ...);
    }
    template<typename... Deps>
    void setupView(Dependencies<Deps...>, dp::thread_pool<>&)
    {
        registry.view<Components..., Deps...>().each([&](Components&... comp, Deps&... deps){
            //pool.enqueue_detach([&](){
                int ret = (accessComponent(deps) + ...);
                assert(ret == 0);
                update((comp,...));
            //});
        });
    }
    void run(dp::thread_pool<>& pool)
    {
        setupView(mergeDependencies((getDependencies<Components>(),...)), pool);
    }
    virtual void update(Components&... components) = 0;
private:
    entt::registry& registry;
};
} // namespace System
} // namespace Seele
