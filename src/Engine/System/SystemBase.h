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
    template<is_component Comp>
    auto getDependencies()
    {
        return Comp::dependencies;
    }
    template<typename Comp>
    Dependencies<> getDependencies()
    {
        return Dependencies<>();
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
                (accessComponent(deps) + ...);
                update((comp,...));
            //});
        });
    }
    template<>
    void setupView(Dependencies<>, dp::thread_pool<>&)
    {
        registry.view<Components...>().each([&](Components&... comp){
            //pool.enqueue_detach([&](){
                update(comp...);
            //});
        });
    }
    void run(dp::thread_pool<>& pool, double delta)
    {
        deltaTime = delta;
        setupView(mergeDependencies((getDependencies<Components>(),...)), pool);
    }
    virtual void update(Components&... components) = 0;
protected:
    double deltaTime;
private:
    entt::registry& registry;
};
} // namespace System
} // namespace Seele
