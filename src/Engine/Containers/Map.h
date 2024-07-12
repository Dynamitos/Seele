#pragma once
#include "Array.h"
#include "Pair.h"
#include "Serialization/Serialization.h"
#include "Tree.h"
#include <utility>


namespace Seele {
template <typename KeyType, typename PairType> struct _KeyFun {
    const KeyType& operator()(const PairType& pair) const { return pair.key; }
};
template <typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::pmr::polymorphic_allocator<Pair<const K, V>>>
struct Map : public Tree<K, Pair<K, V>, _KeyFun<K, Pair<K, V>>, Compare, Allocator> {
    using Super = Tree<K, Pair<K, V>, _KeyFun<K, Pair<K, V>>, Compare, Allocator>;
    using key_type = Super::key_type;
    using mapped_type = V;
    using value_type = Super::value_type;

    using allocator_type = Super::allocator_type;
    using size_type = Super::size_type;
    using difference_type = Super::difference_type;
    using reference = Super::reference;
    using const_reference = Super::reference;
    using pointer = Super::pointer;
    using const_pointer = Super::const_pointer;

    using iterator = Super::iterator;
    using const_iterator = Super::const_iterator;
    using reverse_iterator = Super::reverse_iterator;
    using const_reverse_iterator = Super::const_reverse_iterator;

    constexpr Map() noexcept : Super() {}
    constexpr explicit Map(const Compare& comp, const Allocator& alloc = Allocator()) noexcept(noexcept(Allocator()))
        : Super(comp, alloc) {}
    constexpr explicit Map(const Allocator& alloc) noexcept(noexcept(Compare())) : Super(alloc) {}
    constexpr Map(std::initializer_list<Pair<K, V>> init) : Super(init) {}
    constexpr mapped_type& operator[](const key_type& key) {
        auto [it, inserted] = Super::insert(Pair<K, V>(key, V()));
        return it->value;
    }
    constexpr const mapped_type& operator[](const key_type& key) const { return Super::find(key)->value; }
    constexpr mapped_type& at(const key_type& key) {
        iterator elem = Super::find(key);
        if (elem == Super::end()) {
            throw std::logic_error("Key not found");
        }
        return elem->value;
    }
    constexpr const mapped_type& at(const key_type& key) const { return Super::find(key)->value; }
    constexpr iterator find(const key_type& key) { return Super::find(key); }
    constexpr iterator erase(const key_type& key) { return Super::remove(key); }
    constexpr bool exists(const key_type& key) const { return Super::find(key) != Super::end(); }
    constexpr bool exists(const key_type& key) { return Super::find(key) != Super::end(); }
    constexpr bool contains(const key_type& key) const { return exists(key); }
    constexpr bool contains(const key_type& key) { return exists(key); }
    constexpr Pair<iterator, bool> insert(const value_type& val) { return Super::insert(val); }
    void save(ArchiveBuffer& buffer) const {
        size_t s = Super::size();
        buffer.writeBytes(&s, sizeof(uint64));
        for (const auto& [k, v] : *this) {
            Serialization::save(buffer, k);
            Serialization::save(buffer, v);
        }
    }
    void load(ArchiveBuffer& buffer) {
        uint64 len = 0;
        buffer.readBytes(&len, sizeof(uint64));
        for (uint64 i = 0; i < len; ++i) {
            K k = K();
            V v = V();
            Serialization::load(buffer, k);
            Serialization::load(buffer, v);
            this->operator[](std::move(k)) = std::move(v);
        }
    }
};
} // namespace Seele
