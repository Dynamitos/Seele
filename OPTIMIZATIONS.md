# Build Options

- Compiler optimizations: /O3
- Link Time Optimization: /LTO
- Fast-Math(dangerous)
- Unity Builds
- Profile Guided optimizations
- Static Linking??
- LLVM Bolt

# C++ Options

## Annotations
constexpr
constinit
consteval(C++23)
const
static global variables/functions

## Attributes
\[\[noreturn]] => saves calls
\[\[likely]] => reorders branches
\[\[assume(condition)]] => UB if condition is false
__restrict => disallowes aliasing
\[\[gnu::pure]] => pure math functions, not supported in MSVC

## Avoid copies
using std::optional instead of nullable pointer
use smart pointers
if stealing, use rvalue ref
if need copy, use plain type
if modified, use lvalue ref
if cheap to copy, use plain type
if only reading, use const lvalue ref
if using a range and continous, use std::span
if can be arbitrary range, use std::ranges::***
if it needs to be a specific container, use the container
otherwise take iterator base
use std::string_view(readonly)
for invocables, try:
    - std::invocable<\Args...> auto&& x
    - return_t(*x)(Args...)
    - std::move_only_function&&<\return_t(Args...)> x
    - std::function<\return_t(Args...)> x

## Avoid allocation
dont allocate in loops
dont catch exceptions by value(use const lvalue ref)
use const lvalue refs also in range for loops, lambda captures and structured bindings
provide rvalue accessors T&& value() &&, C++23 provides this auto&& self

# Manual Hardware Optimizations
