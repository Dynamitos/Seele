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
static int accessComponent(Comp& comp);
}
template<typename Comp>
concept has_dependencies = requires(Comp) { Comp::dependencies; };

#define REQUIRE_COMPONENT(x) \
    private: \
    x& get##x() { return *tl_##x; } \
    const x& get##x() const { return *tl_##x; }; \
    public: \
    constexpr static Dependencies<x> dependencies = {};

#define DECLARE_COMPONENT(x) \
    thread_local extern x* tl_##x; \
    template<> \
    int accessComponent<x>(x& val) { tl_##x = &val; return 0; }

#define DEFINE_COMPONENT(x) \
    thread_local x* Seele::Component::tl_##x;

} // namespace Seele
