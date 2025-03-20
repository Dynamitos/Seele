#pragma once

namespace Seele {
template <typename K, typename V> struct Pair {
  public:
    constexpr friend bool operator<(const Pair& left, const Pair& right) {
        return left.key < right.key || (!(right.key < left.key) && left.value < right.value);
    }
    K key;
    V value;
};
} // namespace Seele
