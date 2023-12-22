#pragma once
#include <utility>
#include "Array.h"
#include "Tree.h"

namespace Seele
{
template<class Key, class Compare = std::less<Key>, class Allocator = std::pmr::polymorphic_allocator<Key>>
class Set : public Tree<Key, Key, std::identity, Compare, Allocator>
{
public:
    using Super = Tree<Key, Key, std::identity, Compare, Allocator>;
    using key_type = Key;
    using value_type = Key;
    using node_type = Super::Node;
    using allocator_type = Super::allocator_type;
    using size_type = Super::size_type;
    using difference_type = Super::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;

    using iterator = Super::iterator;
    using const_iterator = Super::const_iterator;
    using reverse_iterator = Super::reverse_iterator;
    using const_reverse_iterator = Super::const_reverse_iterator;

    Set()
        : Set(Compare())
    {
    }
    explicit Set(const Compare& comp, const Allocator alloc = Allocator())
        : Super(comp, alloc)
    {
    }
    explicit Set(const Allocator alloc)
        : Super(alloc)
    {
    }
    constexpr Pair<iterator, bool> insert(const value_type& value)
    {
        return Super::insert(value);
    }
};
} // namespace Seele