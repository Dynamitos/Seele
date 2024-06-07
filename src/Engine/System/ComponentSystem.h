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
    template<typename... Deps>
    void setupView(Dependencies<Deps...>)
    {
        //List<std::function<void()>> work;
        registry.view<Components..., Deps...>().each([&](entt::entity id, Components&... comp, Deps&... deps){
            //work.add([&]() {
                (accessComponent(deps), ...);
                update(comp...);
                update(id, comp...);
            //});
        });
        //getThreadPool().runAndWait(std::move(work));
    }
    virtual void run(double delta) override
    {
        SystemBase::run(delta);
        setupView((getDependencies<Components>() | ...));
    }
    virtual void update() override {}
    virtual void update(Components&... components) {}
    virtual void update(entt::entity id, Components&... components) {}
};
} // namespace System
} // namespace Seele
