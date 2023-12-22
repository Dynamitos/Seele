#pragma once

namespace Seele
{
template <typename K, typename V>
struct Pair
{
public:
    constexpr Pair()
        : key(K()), value(V())
    {}
    constexpr Pair(const K & key, const V & value)
        : key(key), value(value)
    {}
    template<class U1 = K, class U2 = V>
    constexpr Pair(U1&& x, U2&& y)
        : key(std::forward<U1>(x))
        , value(std::forward<U2>(y))
    {
    }
    Pair(const Pair& other) = default;
    Pair(Pair&& other) = default;
    ~Pair(){}
    Pair& operator=(const Pair& other) = default;
    Pair& operator=(Pair&& other) = default;
    constexpr friend bool operator<(const Pair& left, const Pair& right)
    {
        return left.key < right.key || (!(right.key < left.key) && left.value < right.value);
    }
    K key;
    V value;
};
} // namespace Seele
