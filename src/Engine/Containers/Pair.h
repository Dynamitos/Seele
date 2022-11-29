#pragma once

namespace Seele
{
template <typename K, typename V>
struct Pair
{
public:
    Pair()
        : key(K()), value(V())
    {}
    Pair(const Pair& other) = default;
    Pair(Pair&& other) = default;
    ~Pair(){}
    template<class KeyType>
    explicit Pair(KeyType&& key)
        : key(std::forward<KeyType>(key)), value(V())
    {}
    template<class KeyType, class ValueType>
    explicit Pair(KeyType&& key, ValueType&& value)
        : key(std::forward<KeyType>(key)), value(std::forward<ValueType>(value))
    {}
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
