#pragma once
#include <concepts>
#include <tuple>

namespace Seele
{
template<typename... Types>
struct Dependencies;
template<>
struct Dependencies<>
{};
template<typename This, typename... Rest>
struct Dependencies<This, Rest...> : public Dependencies<Rest...>
{
    template<typename... Right>
    Dependencies<This, Rest..., Right...> operator|(const Dependencies<Right...>& other)
    {
        return Dependencies<This, Rest..., Right...>();
    }
};

namespace Component
{
#pragma warning(disable: 4505)
template<typename Comp>
static int accessComponent(Comp& comp);
}

template<typename T, typename... Deps>
concept is_component = std::same_as<decltype(T::dependencies), Dependencies<Deps...>>;

#define REQUIRE_COMPONENT(x) \
    private: \
    x& get##x() { return *tl_##x; } \
    const x& get##x() const { return *tl_##x; }; \
    public: \
    static Dependencies<x> dependencies;

#define DECLARE_COMPONENT(x) \
    thread_local extern x* tl_##x; \
    template<> \
    int accessComponent<x>(x& val) { tl_##x = &val; return 0; }

#define DEFINE_COMPONENT(x) \
    thread_local x* Seele::Component::tl_##x;

} // namespace Seele
