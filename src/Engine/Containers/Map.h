#pragma once
#include <utility>
#include "Array.h"

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
    Pair& operator=(const Pair& other) = default;
    Pair& operator=(Pair&& other) = default;
    template<class KeyType>
    explicit Pair(KeyType&& key)
        : key(std::forward<KeyType>(key)), value(V())
    {}
    template<class KeyType, class ValueType>
    explicit Pair(KeyType&& key, ValueType&& value)
        : key(std::forward<KeyType>(key)), value(std::forward<ValueType>(value))
    {}
    K key;
    V value;
};
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
            : leftChild(-1)
            , rightChild(-1)
            , pair()
        {
        }
        Node(const Node& other) = default;
        Node(Node&& other) = default;
        Node& operator=(const Node& other) = default;
        Node& operator=(Node&& other) = default;
        Node(K key)
            : leftChild(-1)
            , rightChild(-1)
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

        IteratorBase(size_t x = -1)
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
    
    Map()
        : root(-1)
        , beginIt(-1)
        , endIt(-1)
        , iteratorsDirty(true)
        , _size(0)
        , comp(Compare())
    {
    }
    explicit Map(const Compare& comp,
        const Allocator& alloc = Allocator())
        : nodeContainer(alloc)
        , root(-1)
        , beginIt(-1)
        , endIt(-1)
        , iteratorsDirty(true)
        , _size(0)
        , comp(comp)
    {
    }
    explicit Map(const Allocator& alloc)
        : nodeContainer(alloc)
        , root(-1)
        , beginIt(-1)
        , endIt(-1)
        , iteratorsDirty(true)
        , _size(0)
        , comp(Compare())
    {
    }
    Map(const Map& other)
        : nodeContainer(other.nodeContainer)
        , root(other.root)
        , _size(other._size)
        , comp(other.comp)
    {
        markIteratorDirty();
    }
    Map(Map&& other)
        : nodeContainer(other.nodeContainer)
        , root(std::move(other.root))
        , _size(std::move(other._size))
        , comp(std::move(other.comp))
    {
        markIteratorDirty();
    }
    ~Map()
    {
    }
    Map& operator=(const Map& other)
    {
        if(this != &other)
        {
            nodeContainer = other.nodeContainer;
            root = other.root;
            _size = other._size;
            comp = other.comp;
            markIteratorDirty();
        }
        return *this;
    }
    Map& operator=(Map&& other)
    {
        if(this != &other)
        {
            nodeContainer = std::move(other.nodeContainer);
            root = std::move(other.root);
            _size = std::move(other._size);
            comp = std::move(other.comp);
            markIteratorDirty();
        }
        return *this;
    }
    inline mapped_type& operator[](const key_type& key)
    {
        root = splay(root, key);
        markIteratorDirty();
        if (root >= nodeContainer.size() 
         || comp(getNode(root)->pair.key, key) 
         || comp(key, getNode(root)->pair.key))
        {
            root = insert(root, key);
            _size++;
        }
        return getNode(root)->pair.value;
    }
    inline mapped_type& operator[](key_type&& key)
    {
        root = splay(root, std::move(key));
        markIteratorDirty();
        if (root >= nodeContainer.size() 
         || comp(getNode(root)->pair.key, key) 
         || comp(key, getNode(root)->pair.key))
        {
            root = insert(root, std::move(key));
            _size++;
        }
        return getNode(root)->pair.value;
    }
    iterator find(const key_type& key)
    {
        root = splay(root, key);
        refreshIterators();
        if (!isValid(root)
         || comp(getNode(root)->pair.key, key) 
         || comp(key, getNode(root)->pair.key))
        {
            return endIt;
        }
        return iterator(root, &nodeContainer);
    }
    iterator find(key_type&& key)
    {
        root = splay(root, std::move(key));
        refreshIterators();
        if (!isValid(root)
         || comp(getNode(root)->pair.key, key) 
         || comp(key, getNode(root)->pair.key))
        {
            return endIt;
        }
        return iterator(root, &nodeContainer);
    }
    iterator erase(const key_type& key)
    {
        root = remove(root, key);
        refreshIterators();
        return iterator(root, &nodeContainer);	
    }
    iterator erase(K&& key)
    {
        root = remove(root, std::move(key));
        refreshIterators();
        return iterator(root, &nodeContainer);
    }
    void clear()
    {
        nodeContainer.clear();
        root = -1;
        _size = 0;
        markIteratorDirty();
    }
    bool exists(key_type&& key)
    {
        return find(std::forward<K>(key)) != endIt;
    }
    iterator begin()
    {
        if(iteratorsDirty)
        {
            refreshIterators();
        }
        return beginIt;
    }
    iterator end()
    {
        if(iteratorsDirty)
        {
            refreshIterators();
        }
        return endIt;
    }
    iterator begin() const
    {
        if(iteratorsDirty)
        {
            return calcBeginIterator();
        }
        return beginIt;
    }
    iterator end() const
    {
        if(iteratorsDirty)
        {
            return calcEndIterator();
        }
        return endIt;
    }
    bool empty() const
    {
        return nodeContainer.empty();
    }
    size_type size() const
    {
        return _size;
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
    void markIteratorDirty()
    {
        iteratorsDirty = true;
    }
    void refreshIterators()
    {
        beginIt = calcBeginIterator();
        endIt = calcEndIterator();
        iteratorsDirty = false;
    }
    inline Iterator calcBeginIterator() const
    {
        if (!isValid(root))
        {
            return Iterator(-1, &nodeContainer);
        }
        else
        {
            size_t beginIndex = root;
            Array<size_t> beginTraversal;
            while (isValid(beginIndex))
            {
                beginTraversal.add(beginIndex);
                beginIndex = getNode(beginIndex)->leftChild;
            }
            beginIndex = beginTraversal.back();
            beginTraversal.pop();
            return Iterator(beginIndex, &nodeContainer, std::move(beginTraversal));
        }
    }
    inline Iterator calcEndIterator() const
    {
        if (!isValid(root))
        {
            return Iterator(-1, &nodeContainer);
        }
        else
        {
            size_t endIndex = root;
            Array<size_t> endTraversal;
            while (isValid(endIndex))
            {
                endTraversal.add(endIndex);
                endIndex = getNode(endIndex)->rightChild;
            }
            return Iterator(-1, &nodeContainer, std::move(endTraversal));
        }
    }
    Array<Node, NodeAlloc> nodeContainer;
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
            return 0;
        }
        r = splay(r, key);
        Node* node = getNode(r);

        if (!(comp(node->pair.key, key) || comp(key, node->pair.key)))
            return r;

        Node *newNode = &nodeContainer.emplace(std::forward<KeyType>(key));
        node = getNode(r);

        if (comp(key, node->pair.key))
        {
            newNode->rightChild = r;
            newNode->leftChild = node->leftChild;
            node->leftChild = -1;
        }
        else
        {
            newNode->leftChild = r;
            newNode->rightChild = node->rightChild;
            node->rightChild = -1;
        }
        return nodeContainer.size() - 1;
    }
    template<class KeyType>
    size_t remove(size_t r, KeyType&& key)
    {
        size_t temp;
        if (!isValid(r))
            return -1;

        r = splay(r, key);
        Node* node = getNode(r);

        if (comp(node->pair.key, key) || comp(key, node->pair.key))
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
        if (node == nullptr 
         || !(comp(node->pair.key, key) 
          ||  comp(key, node->pair.key)))
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
};
} // namespace Seele