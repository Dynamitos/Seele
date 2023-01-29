#pragma once
#include "SystemBase.h"
#include "Component/Component.h"

namespace Seele
{
namespace System
{
template<typename... Components>
class ComponentSystem : public SystemBase
{
public:
    ComponentSystem(PScene scene) : SystemBase(scene) {}
    virtual ~ComponentSystem() {}
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
    virtual void run(dp::thread_pool<>& pool, double delta) override
    {
        SystemBase::run(pool, delta);
        setupView(mergeDependencies((getDependencies<Components>(),...)), pool);
    }
    virtual void update(Components&... components) = 0;
};
} // namespace System
} // namespace Seele
