#pragma once
#include <concepts>

namespace Seele
{
template<class F, class... Args>
concept invocable = std::invocable<F, Args...>;

template<class T, class Archive>
concept serializable = requires(T t, Archive& a)
{
    t->save(a);
    t->load(a);
};
template<typename T>
concept enumeration = std::is_enum_v<T>;
}