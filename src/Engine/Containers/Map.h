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
        size_t leftChild;
        size_t rightChild;
        Pair<K, V> pair;
        Node()
            : leftChild(SIZE_MAX)
            , rightChild(SIZE_MAX)
            , pair()
        {
        }
        Node(const Node& other) = default;
        Node(Node&& other) = default;
        Node& operator=(const Node& other) = default;
        Node& operator=(Node&& other) = default;
        Node(K key)
            : leftChild(SIZE_MAX)
            , rightChild(SIZE_MAX)
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

        IteratorBase(size_t x = SIZE_MAX)
            : node(x)
        {
        }
        IteratorBase(size_t x, const Array<Node, NodeAlloc>* nodeContainer, Array<size_t> &&beginIt = Array<size_t>())
            : node(x), traversal(std::move(beginIt)), nodeContainer(nodeContainer)
        {
        }
        IteratorBase(const IteratorBase &i)
            : node(i.node), traversal(i.traversal), nodeContainer(i.nodeContainer)
        {
        }
        IteratorBase(IteratorBase&& i)
            : node(std::move(i.node)), traversal(std::move(i.traversal)), nodeContainer(i.nodeContainer)
        {
        }
        IteratorBase& operator=(const IteratorBase& other)
        {
            if(this != &other)
            {
                node = other.node;
                nodeContainer = other.nodeContainer;
                traversal = other.traversal;
            }
            return *this;
        }
        IteratorBase& operator=(IteratorBase&& other)
        {
            if(this != &other)
            {
                node = std::move(other.node);
                nodeContainer = std::move(other.nodeContainer);
                traversal = std::move(other.traversal);
            }
            return *this;
        }
        reference operator*() const
        {
            return getNode()->pair;
        }
        pointer operator->() const
        {
            return &(getNode()->pair);
        }
        inline bool operator!=(const IteratorBase &other)
        {
            return node != other.node;
        }
        inline bool operator==(const IteratorBase &other)
        {
            return node == other.node;
        }
        IteratorBase &operator++()
        {
            node = getNode()->rightChild;
            while (node < nodeContainer->size() 
                && getNode()->leftChild < nodeContainer->size())
            {
                traversal.add(node);
                node = getNode()->leftChild;
            }
            if (node >= nodeContainer->size() 
                && traversal.size() > 0)
            {
                node = traversal.back();
                traversal.pop();
            }
            return *this;
        }
        IteratorBase &operator--()
        {
            node = getNode()->leftChild;
            while (node < nodeContainer->size() 
                && getNode()->rightChild < nodeContainer->size())
            {
                traversal.add(node);
                node = getNode()->rightchild;
            }
            if (node >= nodeContainer->size() 
                && traversal.size() > 0)
            {
                node = traversal.back();
                traversal.pop();
            }
            return *this;
        }
        IteratorBase operator--(int)
        {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }
        IteratorBase operator++(int)
        {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }

    private:
        Node* getNode() const
        {
            return &(*nodeContainer)[node];
        }
        size_t node;
        Array<size_t> traversal;
        const Array<Node, NodeAlloc>* nodeContainer;
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
        : root(SIZE_MAX)
        , beginIt(SIZE_MAX)
        , endIt(SIZE_MAX)
        , iteratorsDirty(true)
        , _size(0)
        , comp(Compare())
    {
    }
    constexpr explicit Map(const Compare& comp, const Allocator& alloc = Allocator()) noexcept(noexcept(Allocator()))
        : nodeContainer(alloc)
        , root(SIZE_MAX)
        , beginIt(SIZE_MAX)
        , endIt(SIZE_MAX)
        , iteratorsDirty(true)
        , _size(0)
        , comp(comp)
    {
    }
    constexpr explicit Map(const Allocator& alloc) noexcept(noexcept(Compare()))
        : nodeContainer(alloc)
        , root(SIZE_MAX)
        , beginIt(SIZE_MAX)
        , endIt(SIZE_MAX)
        , iteratorsDirty(true)
        , _size(0)
        , comp(Compare())
    {
    }
    constexpr Map(const Map& other)
        : nodeContainer(other.nodeContainer)
        , root(other.root)
        , iteratorsDirty(true)
        , _size(other._size)
        , comp(other.comp)
    {
    }
    constexpr Map(Map&& other) noexcept
        : nodeContainer(std::move(other.nodeContainer))
        , root(std::move(other.root))
        , iteratorsDirty(true)
        , _size(std::move(other._size))
        , comp(std::move(other.comp))
    {
    }
    constexpr ~Map() noexcept
    {
    }
    constexpr Map& operator=(const Map& other)
    {
        if(this != &other)
        {
            nodeContainer = other.nodeContainer;
            root = other.root;
            _size = other._size;
            comp = other.comp;
            markIteratorsDirty();
        }
        return *this;
    }
    constexpr Map& operator=(Map&& other)
    {
        if(this != &other)
        {
            nodeContainer = std::move(other.nodeContainer);
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
        if (!isValid(root) || !equal(getNode(root)->pair.key, key))
        {
            root = insert(root, key);
            _size++;
        }
        markIteratorsDirty();
        return getNode(root)->pair.value;
    }
    constexpr const mapped_type& operator[](const key_type& key) const
    {
        size_t it = root;
        while (!equal(getNode(it)->pair.key, key))
        {
            if (comp(key, getNode(it)->pair.key))
            {
                it = getNode(it)->leftChild;
            }
            else
            {
                it = getNode(it)->rightChild;
            }
        }
        return getNode(it)->pair.value;
    }
    constexpr mapped_type& at(const key_type& key)
    {
        root = splay(root, key);
        if (!isValid(root) || !equal(getNode(root)->pair.key, key))
        {
            throw std::logic_error("Key not found");
        }
        markIteratorsDirty();
        return getNode(root)->pair.value;
    }
    constexpr mapped_type& at(key_type&& key)
    {
        root = splay(root, std::move(key));
        if (!isValid(root) || !equal(getNode(root)->pair.key, key))
        {
            throw std::logic_error("Key not found");
        }
        markIteratorsDirty();
        return getNode(root)->pair.value;
    }
    
    constexpr const mapped_type& at(const key_type& key) const
    {
        for(const auto& node : nodeContainer)
        {
            if(equal(node.pair.key, key))
            {
                return node.pair.value;
            }
        }
        throw std::logic_error("Key not found");
    }
    constexpr iterator find(const key_type& key)
    {
        root = splay(root, key);
        refreshIterators();
        if (!isValid(root) || !equal(getNode(root)->pair.key, key))
        {
            return endIt;
        }
        return iterator(root, &nodeContainer);
    }
    constexpr iterator find(key_type&& key)
    {
        root = splay(root, std::move(key));
        refreshIterators();
        if (!isValid(root) || !equal(getNode(root)->pair.key, key))
        {
            return endIt;
        }
        return iterator(root, &nodeContainer);
    }
    constexpr iterator erase(const key_type& key)
    {
        root = remove(root, key);
        refreshIterators();
        assert(root != SIZE_MAX || _size == 0);
        return iterator(root, &nodeContainer);	
    }
    constexpr iterator erase(key_type&& key)
    {
        root = remove(root, std::move(key));
        refreshIterators();
        return iterator(root, &nodeContainer);
    }
    constexpr void clear()
    {
        nodeContainer.clear();
        root = SIZE_MAX;
        _size = 0;
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
        return nodeContainer.empty();
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
        for(size_t i = 0; i < nodeContainer.size(); ++i)
        {
            bool found = false;
            for(auto it : nodeContainer)
            {
                if(it.leftChild == i)
                {
                    assert(!found);
                    found = true;
                }
                if(it.rightChild == i)
                {
                    assert(!found);
                    found = true;
                }
            }
            if(isValid(nodeContainer[i].leftChild))
            {
                assert(comp(nodeContainer[nodeContainer[i].leftChild].pair.key, nodeContainer[i].pair.key));
            }
            if(isValid(nodeContainer[i].rightChild))
            {
                assert(comp(nodeContainer[i].pair.key, nodeContainer[nodeContainer[i].rightChild].pair.key));
            }
        }
    }
    Node* getNode(size_t index) const
    {
        if(!isValid(index)) return nullptr;
        return &nodeContainer[index];
    }
    inline bool isValid(size_t index) const
    {
        return index < nodeContainer.size();
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
        size_t beginIndex = root;
        Array<size_t> beginTraversal;
        while (isValid(beginIndex))
        {
            beginTraversal.add(beginIndex);
            beginIndex = getNode(beginIndex)->leftChild;
        }
        if(!beginTraversal.empty())
        {
            beginIndex = beginTraversal.back();
            beginTraversal.pop();
        }
        return Iterator(beginIndex, &nodeContainer, std::move(beginTraversal));
    }
    constexpr Iterator calcEndIterator() const
    {
        size_t endIndex = root;
        Array<size_t> endTraversal;
        while (isValid(endIndex))
        {
            endTraversal.add(endIndex);
            endIndex = getNode(endIndex)->rightChild;
        }
        return Iterator(endIndex, &nodeContainer, std::move(endTraversal));
    }
    Array<Node, NodeAlloc> nodeContainer = Array<Node, NodeAlloc>();
    size_t root;
    Iterator beginIt;
    Iterator endIt;
    bool iteratorsDirty;
    size_type _size;
    Compare comp;
    size_t rotateRight(size_t node)
    {
        Node* x = getNode(node);
        size_t res = x->leftChild;
        Node* y = getNode(x->leftChild);
        x->leftChild = y->rightChild;
        y->rightChild = node;
        return res;
    }
    size_t rotateLeft(size_t node)
    {
        Node* x = getNode(node);
        size_t res = x->rightChild;
        Node* y = getNode(x->rightChild);
        x->rightChild = y->leftChild;
        y->leftChild = node;
        return res;
    }
    template<class KeyType>
    size_t insert(size_t r, KeyType&& key)
    {
        if (!isValid(r))
        {
            nodeContainer.emplace(std::forward<KeyType>(key));
            return nodeContainer.size() - 1;
        }
        r = splay(r, key);
        Node* node = getNode(r);

        if (equal(node->pair.key, key))
            return r;

        Node *newNode = &nodeContainer.emplace(std::forward<KeyType>(key));
        node = getNode(r);

        if (comp(newNode->pair.key, node->pair.key))
        {
            newNode->rightChild = r;
            newNode->leftChild = node->leftChild;
            node->leftChild = SIZE_MAX;
        }
        else
        {
            newNode->leftChild = r;
            newNode->rightChild = node->rightChild;
            node->rightChild = SIZE_MAX;
        }
        return nodeContainer.size() - 1;
    }
    template<class KeyType>
    size_t remove(size_t r, KeyType&& key)
    {
        size_t temp;
        if (!isValid(r))
            return SIZE_MAX;

        r = splay(r, key);
        Node* node = getNode(r);

        if (!equal(node->pair.key, key))
            return r;

        if (!isValid(node->leftChild))
        {
            temp = r;
            r = node->rightChild;
        }
        else
        {
            temp = r;

            r = splay(node->leftChild, key);
            node = getNode(r);
            node->rightChild = getNode(temp)->rightChild;
        }
        Node& lastNode = nodeContainer.back();
        size_t lastIndex = nodeContainer.size() - 1;
        size_t removedIndex = temp;
        //Arrays can only pop back, so we need to move the last element to the deleted index
        if(removedIndex != lastIndex)
        {
            nodeContainer[removedIndex] = std::move(lastNode);
            for(auto& it : nodeContainer)
            {
                if(it.leftChild == lastIndex)
                {
                    it.leftChild = removedIndex;
                }
                if(it.rightChild == lastIndex)
                {
                    it.rightChild = removedIndex;
                }
            }
        }
        nodeContainer.pop();
        _size--;
        return r;
    }
    template<class KeyType>
    size_t splay(size_t r, KeyType&& key)
    {
        Node* node = getNode(r);
        if (node == nullptr || equal(node->pair.key, key))
        {
            return r;
        }
        if (comp(key, node->pair.key))
        {
            if (!isValid(node->leftChild))
                return r;

            if (comp(key, getNode(node->leftChild)->pair.key))
            {
                getNode(node->leftChild)->leftChild = splay(getNode(node->leftChild)->leftChild, key);

                r = rotateRight(r);
                node = getNode(r);
            }
            else if (comp(getNode(node->leftChild)->pair.key, key))
            {
                getNode(node->leftChild)->rightChild = splay(getNode(node->leftChild)->rightChild, key);

                if (isValid(getNode(node->leftChild)->rightChild))
                {
                    node->leftChild = rotateLeft(node->leftChild);
                }
            }
            return (!isValid(node->leftChild)) ? r : rotateRight(r);
        }
        else
        {
            if (!isValid(node->rightChild))
                return r;

            if (comp(key, getNode(node->rightChild)->pair.key))
            {
                getNode(node->rightChild)->leftChild = splay(getNode(node->rightChild)->leftChild, key);

                if (isValid(getNode(node->rightChild)->leftChild))
                {
                    node->rightChild = rotateRight(node->rightChild);
                }
            }
            else if (comp(getNode(node->rightChild)->pair.key, key))
            {
                getNode(node->rightChild)->rightChild = splay(getNode(node->rightChild)->rightChild, key);

                r = rotateLeft(r);
                node = getNode(r);
            }
            return (!isValid(node->rightChild)) ? r : rotateLeft(r);
        }
    }
    bool equal(const key_type& a, const key_type& b) const
    {
        return !comp(a, b) && !comp(b, a);
    }
};
} // namespace Seele
