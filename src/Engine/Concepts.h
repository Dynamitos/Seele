#pragma once
#include <concepts>

namespace Seele
{
template<class F, class... Args>
concept invocable = std::invocable<F, Args...>;
}