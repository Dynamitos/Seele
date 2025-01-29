#pragma once
#include <concepts>
#include <ranges>

namespace Seele {

template <typename Range, typename T>
concept container_compatible_range = (std::ranges::input_range<Range>) && std::convertible_to<std::ranges::range_reference_t<Range>, T>;

template <class F, class... Args>
concept invocable = std::invocable<F, Args...>;

template <class Archive, class T>
concept serializable = requires(const T& t, Archive& a) { t.save(a); } && requires(T& t, Archive& a) { t.load(a); };
template <typename T>
concept enumeration = std::is_enum_v<T>;
} // namespace Seele