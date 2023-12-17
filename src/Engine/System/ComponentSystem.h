#pragma once
#include "SystemBase.h"
#include "Component/Component.h"
#include "Component/Camera.h"

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
    template<has_dependencies Comp>
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
    void setupView(Dependencies<>, ThreadPool& pool)
    {
        List<std::function<void()>> work;
        registry.view<Components...>().each([&](Components&... comp){
            work.add([&](){
                update(comp...);
            });
        });
        pool.runAndWait(std::move(work));
    }
    template<typename... Deps>
    void setupView(Dependencies<Deps...>, ThreadPool& pool)
    {
        List<std::function<void()>> work;
        registry.view<Components..., Deps...>().each([&](Components&... comp, Deps&... deps){
            work.add([&]() {
                (accessComponent(deps) + ...);
                update((comp,...));
            });
        });
        pool.runAndWait(std::move(work));
    }
    virtual void run(ThreadPool& pool, double delta) override
    {
        SystemBase::run(pool, delta);
        setupView(mergeDependencies((getDependencies<Components>(),...)), pool);
    }
    virtual void update() {}
    virtual void update(Components&... components) = 0;
};
} // namespace System
} // namespace Seele
