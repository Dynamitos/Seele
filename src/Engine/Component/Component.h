#pragma once
#include <concepts>
#include <tuple>

namespace Seele
{
template<typename... Types>
struct Dependencies;
template<>
struct Dependencies<>
{
    int x;
    template<typename... Right>
    Dependencies<Right...> operator|(const Dependencies<Right...>& other)
    {
        return Dependencies<Right...>();
    }
};
template<typename This, typename... Rest>
struct Dependencies<This, Rest...> : public Dependencies<Rest...>
{
    template<typename... Right>
    Dependencies<This, Rest..., Right...> operator|(const Dependencies<Right...>& other)
    {
        return Dependencies<This, Rest..., Right...>();
    }
    int x;
};

namespace Component
{
    template<typename Comp>
    Comp& getComponent();
}

template<typename Comp>
concept has_dependencies = requires(Comp) { Comp::dependencies; };

#define REQUIRE_COMPONENT(x) \
    private: \
    x& get##x() { return getComponent<##x>(); } \
    const x& get##x() const { return getComponent<##x>(); } \
    public: \
    constexpr static Dependencies<x> dependencies = {};

#define DECLARE_COMPONENT(x) \
    void accessComponent(x& val); \
    template<> \
    x& getComponent<x>();
    

#define DEFINE_COMPONENT(x) \
    thread_local x* tl_##x = nullptr; \
    void Seele::Component::accessComponent(x& ref) { tl_##x = &ref; } \
    template<> \
    x& Seele::Component::getComponent<x>() { return *tl_##x; }


} // namespace Seele
