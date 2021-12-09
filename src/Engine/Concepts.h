#pragma once
#include <concepts>

namespace Seele
{
template<typename A>
concept iterable = requires(A&& a)
{
    a.begin(); a.end();
};
template<class F, class... Args>
concept invocable = std::invocable<F, Args...>;
}