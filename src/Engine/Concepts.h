#pragma once
#include <concepts>

namespace Seele {
template <class F, class... Args>
concept invocable = std::invocable<F, Args...>;

template <class Archive, class T>
concept serializable = requires(const T& t, Archive& a) { t.save(a); } && requires(T& t, Archive& a) { t.load(a); };
template <typename T>
concept enumeration = std::is_enum_v<T>;
} // namespace Seele