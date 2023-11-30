#pragma once
#include <utility>
#include "Array.h"
#include "Pair.h"
#include "Serialization/Serialization.h"

namespace Seele
{
template <typename K, 
          typename V, 
          typename Compare = std::less<K>,
          typename Allocator = std::pmr::polymorphic_allocator<Pair<const K,V>>>
struct Map
{
private:
    struct Node
    {
        Node* leftChild;
        Node* rightChild;
        Pair<K, V> pair;
        Node()
            : leftChild(nullptr)
            , rightChild(nullptr)
            , pair()
        {
        }
        Node(const Node& other) = default;
        Node(Node&& other) = default;
        Node& operator=(const Node& other) = default;
        Node& operator=(Node&& other) = default;
        Node(K key)
            : leftChild(nullptr)
            , rightChild(nullptr)
            , pair(std::move(key))
        {
        }
        ~Node()
        {
        }
    };
    using NodeAlloc = std::allocator_traits<Allocator>::template rebind_alloc<Node>;
public:
    template<typename PairType>
    class IteratorBase
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = PairType;
        using difference_type = std::ptrdiff_t;
        using reference = PairType&;
        using pointer = PairType*;

        constexpr IteratorBase(Node* x = nullptr, Array<Node*> &&beginIt = Array<Node*>())
            : node(x), traversal(std::move(beginIt))
        {
        }
        constexpr IteratorBase(const IteratorBase &i)
            : node(i.node), traversal(i.traversal)
        {
        }
        constexpr IteratorBase(IteratorBase&& i)
            : node(std::move(i.node)), traversal(std::move(i.traversal))
        {
        }
        constexpr IteratorBase& operator=(const IteratorBase& other)
        {
            if(this != &other)
            {
                node = other.node;
                traversal = other.traversal;
            }
            return *this;
        }
        constexpr IteratorBase& operator=(IteratorBase&& other)
        {
            if(this != &other)
            {
                node = std::move(other.node);
                traversal = std::move(other.traversal);
            }
            return *this;
        }
        constexpr reference operator*() const
        {
            return node->pair;
        }
        constexpr pointer operator->() const
        {
            return &(node->pair);
        }
        constexpr bool operator!=(const IteratorBase &other)
        {
            return node != other.node;
        }
        constexpr bool operator==(const IteratorBase &other)
        {
            return node == other.node;
        }
        constexpr IteratorBase &operator++()
        {
            node = node->rightChild;
            while (node != nullptr
                && node->leftChild != nullptr)
            {
                traversal.add(node);
                node = node->leftChild;
            }
            if (node == nullptr 
                && traversal.size() > 0)
            {
                node = traversal.back();
                traversal.pop();
            }
            return *this;
        }
        constexpr IteratorBase &operator--()
        {
            node = node->leftChild;
            while (node != nullptr 
                && node->rightChild != nullptr)
            {
                traversal.add(node);
                node = node->rightchild;
            }
            if (node == nullptr 
                && traversal.size() > 0)
            {
                node = traversal.back();
                traversal.pop();
            }
            return *this;
        }
        constexpr IteratorBase operator--(int)
        {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }
        constexpr IteratorBase operator++(int)
        {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }

    private:
        Node* node;
        Array<Node*> traversal;
    };
    using Iterator = IteratorBase<Pair<K,V>>;
    using ConstIterator = IteratorBase<const Pair<K,V>>;
    
    using key_type = K;
    using mapped_type = V;
    using value_type = Pair<K,V>;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;

    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    constexpr Map() noexcept
        : root(nullptr)
        , beginIt(nullptr)
        , endIt(nullptr)
        , iteratorsDirty(true)
        , _size(0)
        , comp(Compare())
    {
    }
    constexpr explicit Map(const Compare& comp, const Allocator& alloc = Allocator()) noexcept(noexcept(Allocator()))
        : alloc(alloc)
        , root(nullptr)
        , beginIt(nullptr)
        , endIt(nullptr)
        , iteratorsDirty(true)
        , _size(0)
        , comp(comp)
    {
    }
    constexpr explicit Map(const Allocator& alloc) noexcept(noexcept(Compare()))
        : alloc(alloc)
        , root(nullptr)
        , beginIt(nullptr)
        , endIt(nullptr)
        , iteratorsDirty(true)
        , _size(0)
        , comp(Compare())
    {
    }
    constexpr Map(const Map& other)
        : alloc(other.alloc)
        , root(nullptr)
        , iteratorsDirty(true)
        , _size(0)
        , comp(other.comp)
    {
        for(const auto& [key, val] : other)
        {
            this->operator[](key) = val;
        }
    }
    constexpr Map(Map&& other) noexcept
        : alloc(std::move(other.alloc))
        , root(std::move(other.root))
        , iteratorsDirty(true)
        , _size(std::move(other._size))
        , comp(std::move(other.comp))
    {
    }
    constexpr ~Map() noexcept
    {
        clear();
    }
    constexpr Map& operator=(const Map& other)
    {
        if(this != &other)
        {
            alloc = other.alloc;
            comp = other.comp;
            root = nullptr;
            _size = 0;
            for(const auto& [key, val] : other)
            {
                this->operator[](key) = val;
            }
            markIteratorsDirty();
        }
        return *this;
    }
    constexpr Map& operator=(Map&& other)
    {
        if(this != &other)
        {
            alloc = std::move(other.alloc);
            root = std::move(other.root);
            _size = std::move(other._size);
            comp = std::move(other.comp);
            markIteratorsDirty();
        }
        return *this;
    }
    constexpr mapped_type& operator[](const key_type& key)
    {
        root = splay(root, key);
        if (root == nullptr || !equal(root->pair.key, key))
        {
            root = insert(root, key);
            _size++;
        }
        markIteratorsDirty();
        return root->pair.value;
    }
    constexpr const mapped_type& operator[](const key_type& key) const
    {
        Node* it = root;
        while (!equal(it->pair.key, key))
        {
            if (comp(key, it->pair.key))
            {
                it = it->leftChild;
            }
            else
            {
                it = it->rightChild;
            }
        }
        return it->pair.value;
    }
    constexpr mapped_type& at(const key_type& key)
    {
        root = splay(root, key);
        if (root == nullptr || !equal(root->pair.key, key))
        {
            throw std::logic_error("Key not found");
        }
        markIteratorsDirty();
        return root->pair.value;
    }
    constexpr mapped_type& at(key_type&& key)
    {
        root = splay(root, std::move(key));
        if (root == nullptr || !equal(root->pair.key, key))
        {
            throw std::logic_error("Key not found");
        }
        markIteratorsDirty();
        return root->pair.value;
    }
    
    constexpr const mapped_type& at(const key_type& key) const
    {
        Node* it = root;
        while (!equal(it->pair.key, key))
        {
            if (comp(key, it->pair.key))
            {
                it = it->leftChild;
            }
            else
            {
                it = it->rightChild;
            }
        }
        return it->pair.value;
    }
    constexpr iterator find(const key_type& key)
    {
        root = splay(root, key);
        refreshIterators();
        if (root == nullptr || !equal(root->pair.key, key))
        {
            return endIt;
        }
        return iterator(root);
    }
    constexpr iterator find(key_type&& key)
    {
        root = splay(root, std::move(key));
        refreshIterators();
        if (root == nullptr || !equal(root->pair.key, key))
        {
            return endIt;
        }
        return iterator(root);
    }
    constexpr iterator erase(const key_type& key)
    {
        root = remove(root, key);
        refreshIterators();
        assert(root != nullptr || _size == 0);
        return iterator(root);	
    }
    constexpr iterator erase(key_type&& key)
    {
        root = remove(root, std::move(key));
        refreshIterators();
        return iterator(root);
    }
    constexpr void clear()
    {
        while(_size > 0)
        {
            root = remove(root, root->pair.key);
        }
        root = nullptr;
        markIteratorsDirty();
    }
    constexpr bool exists(const key_type& key)
    {
        return find(key) != endIt;
    }
    constexpr bool exists(key_type&& key)
    {
        return find(std::move(key)) != endIt;
    }
    constexpr bool contains(const key_type& key)
    {
        return exists(key);
    }
    constexpr bool contains(key_type&& key)
    {
        return exists(key);
    }
    constexpr iterator begin()
    {
        if(iteratorsDirty)
        {
            refreshIterators();
        }
        return beginIt;
    }
    constexpr iterator end()
    {
        if(iteratorsDirty)
        {
            refreshIterators();
        }
        return endIt;
    }
    constexpr iterator begin() const
    {
        if(iteratorsDirty)
        {
            return calcBeginIterator();
        }
        return beginIt;
    }
    constexpr iterator end() const
    {
        if(iteratorsDirty)
        {
            return calcEndIterator();
        }
        return endIt;
    }
    constexpr bool empty() const
    {
        return _size == 0;
    }
    constexpr size_type size() const
    {
        return _size;
    }
    void save(ArchiveBuffer& buffer) const
    {
        buffer.writeBytes(&_size, sizeof(uint64));
        for(const auto& [k, v] : *this)
        {
            Serialization::save(buffer, k);
            Serialization::save(buffer, v);
        }
    }
    void load(ArchiveBuffer& buffer)
    {
        uint64 len = 0;
        buffer.readBytes(&len, sizeof(uint64));
        for(uint64 i = 0; i < len; ++i)
        {
            K k = K();
            V v = V();
            Serialization::load(buffer, k);
            Serialization::load(buffer, v);
            this->operator[](std::move(k)) = std::move(v);
        }
    }
private:
    void verifyTree()
    {
        size_t numElems = 0;
        for (const auto& it : *this)
        {
            numElems++;
        }
        assert(numElems == _size);
    }
    void markIteratorsDirty()
    {
        iteratorsDirty = true;
    }
    void refreshIterators()
    {
        beginIt = calcBeginIterator();
        endIt = calcEndIterator();
        iteratorsDirty = false;
    }
    constexpr Iterator calcBeginIterator() const
    {
        Node* begin = root;
        Array<Node*> beginTraversal;
        while (begin != nullptr)
        {
            beginTraversal.add(begin);
            begin = begin->leftChild;
        }
        if(!beginTraversal.empty())
        {
            begin = beginTraversal.back();
            beginTraversal.pop();
        }
        return Iterator(begin, std::move(beginTraversal));
    }
    constexpr Iterator calcEndIterator() const
    {
        Node* endIndex = root;
        Array<Node*> endTraversal;
        while (endIndex != nullptr)
        {
            endTraversal.add(endIndex);
            endIndex = endIndex->rightChild;
        }
        return Iterator(endIndex, std::move(endTraversal));
    }
    NodeAlloc alloc;
    Node* root;
    Iterator beginIt;
    Iterator endIt;
    bool iteratorsDirty;
    size_type _size;
    Compare comp;
    Node* rotateLeft(Node* x)
    {
        Node* y = x->rightChild;
        x->rightChild = y->leftChild;
        y->leftChild = x;
        return y;
    }
    Node* rotateRight(Node* x)
    {
        Node* y = x->leftChild;
        x->leftChild = y->rightChild;
        y->rightChild = x;
        return y;
    }
    template<class KeyType>
    Node* insert(Node* r, KeyType&& key)
    {
        if (r == nullptr)
        {
            root = alloc.allocate(1);
            std::allocator_traits<NodeAlloc>::construct(alloc, root, std::forward<KeyType>(key));
            return root;
        }
        r = splay(r, key);

        if (equal(r->pair.key, key))
            return r;

        Node *newNode = alloc.allocate(1);
        std::allocator_traits<NodeAlloc>::construct(alloc, newNode, std::forward<KeyType>(key));

        if (comp(newNode->pair.key, r->pair.key))
        {
            newNode->rightChild = r;
            newNode->leftChild = r->leftChild;
            r->leftChild = nullptr;
        }
        else
        {
            newNode->leftChild = r;
            newNode->rightChild = r->rightChild;
            r->rightChild = nullptr;
        }
        return newNode;
    }
    template<class KeyType>
    Node* remove(Node* r, KeyType&& key)
    {
        Node* temp;
        if (r == nullptr)
            return nullptr;

        r = splay(r, key);

        if (!equal(r->pair.key, key))
            return r;

        if (r->leftChild == nullptr)
        {
            temp = r;
            r = r->rightChild;
        }
        else
        {
            temp = r;

            r = splay(r->leftChild, key);
            r->rightChild = temp->rightChild;
        }
        std::allocator_traits<NodeAlloc>::destroy(alloc, temp);
        alloc.deallocate(temp, 1);
        _size--;
        return r;
    }
    template<class KeyType>
    Node* splay(Node* r, KeyType&& key)
    {
        if (r == nullptr || equal(r->pair.key, key))
        {
            return r;
        }
        if (comp(key, r->pair.key))
        {
            if (r->leftChild == nullptr)
                return r;

            if (comp(key, r->leftChild->pair.key))
            {
                r->leftChild->leftChild = splay(r->leftChild->leftChild, key);

                r = rotateRight(r);
            }
            else if (comp(r->leftChild->pair.key, key))
            {
                r->leftChild->rightChild = splay(r->leftChild->rightChild, key);

                if (r->leftChild->rightChild != nullptr)
                {
                    r->leftChild = rotateLeft(r->leftChild);
                }
            }
            return (r->leftChild == nullptr) ? r : rotateRight(r);
        }
        else
        {
            if (r->rightChild == nullptr)
                return r;

            if (comp(key, r->rightChild->pair.key))
            {
                r->rightChild->leftChild = splay(r->rightChild->leftChild, key);

                if (r->rightChild->leftChild != nullptr)
                {
                    r->rightChild = rotateRight(r->rightChild);
                }
            }
            else if (comp(r->rightChild->pair.key, key))
            {
                r->rightChild->rightChild = splay(r->rightChild->rightChild, key);

                r = rotateLeft(r);
            }
            return (r->rightChild == nullptr) ? r : rotateLeft(r);
        }
    }
    bool equal(const key_type& a, const key_type& b) const
    {
        return !comp(a, b) && !comp(b, a);
    }
};
} // namespace Seele
