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
        Node *leftChild;
        Node *rightChild;
        Pair<K, V> pair;
        Node()
            : leftChild(nullptr), rightChild(nullptr), pair()
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

        IteratorBase(Node *x = nullptr)
            : node(x)
        {
        }
        IteratorBase(Node *x, Array<Node *> &&beginIt)
            : node(x), traversal(std::move(beginIt))
        {
        }
        IteratorBase(const IteratorBase &i)
            : node(i.node), traversal(i.traversal)
        {
        }
        IteratorBase(IteratorBase&& i)
            : node(std::move(i.node)), traversal(std::move(i.traversal))
        {
        }
        IteratorBase& operator=(const IteratorBase& other)
        {
            if(this != &other)
            {
                node = other.node; // No copy, since no ownership
                traversal = other.traversal;
            }
            return *this;
        }
        IteratorBase& operator=(IteratorBase&& other)
        {
            if(this != &other)
            {
                node = std::move(other.node);
                traversal = std::move(other.traversal);
            }
            return *this;
        }
        reference operator*() const
        {
            return node->pair;
        }
        pointer operator->() const
        {
            return &node->pair;
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
            node = node->rightChild;
            while (node != nullptr && node->leftChild != nullptr)
            {
                traversal.add(node);
                node = node->leftChild;
            }
            if (node == nullptr && traversal.size() > 0)
            {
                node = traversal.back();
                traversal.pop();
            }
            return *this;
        }
        IteratorBase &operator--()
        {
            node = node->leftChild;
            while (node != nullptr && node->rightchild != nullptr)
            {
                traversal.add(node);
                node = node->rightChild;
            }
            if (node == nullptr && traversal.size() > 0)
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
        Node *node;
        Array<Node *> traversal;
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
        : root(nullptr)
        , beginIt(nullptr)
        , endIt(nullptr)
        , iteratorsDirty(true)
        , _size(0)
        , comp(Compare())
    {
    }
    explicit Map(const Compare& comp,
        const Allocator& alloc = Allocator())
        : nodeContainer(alloc)
        , root(nullptr)
        , beginIt(nullptr)
        , endIt(nullptr)
        , iteratorsDirty(true)
        , _size(0)
        , comp(comp)
    {
    }
    explicit Map(const Allocator& alloc)
        : nodeContainer(alloc)
        , root(nullptr)
        , beginIt(nullptr)
        , endIt(nullptr)
        , iteratorsDirty(true)
        , _size(0)
        , comp(Compare())
    {
    }
    Map(const Map& other)
        : nodeContainer(other.nodeContainer)
        , _size(other._size)
        , comp(other.comp)
    {
        root = &nodeContainer[nodeContainer.indexOf(other.root)];
        markIteratorDirty();
    }
    Map(Map&& other)
        : nodeContainer(other.nodeContainer)
        , _size(std::move(other._size))
        , comp(std::move(other.comp))
    {
        root = &nodeContainer[nodeContainer.indexOf(other.root)];
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
            root = &nodeContainer[nodeContainer.indexOf(other.root)];
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
            root = &nodeContainer[nodeContainer.indexOf(other.root)];
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
        if (root == nullptr || comp(root->pair.key, key) || comp(key, root->pair.key))
        {
            root = insert(root, key);
            _size++;
        }
        return root->pair.value;
    }
    inline mapped_type& operator[](key_type&& key)
    {
        root = splay(root, std::move(key));
        markIteratorDirty();
        if (root == nullptr || comp(root->pair.key, key) || comp(key, root->pair.key))
        {
            root = insert(root, std::move(key));
            _size++;
        }
        return root->pair.value;
    }
    iterator find(const key_type& key)
    {
        root = splay(root, key);
        markIteratorDirty();
        if (root == nullptr || comp(root->pair.key, key) || comp(key, root->pair.key))
        {
            return endIt;
        }
        return iterator(root);
    }
    iterator find(key_type&& key)
    {
        root = splay(root, std::move(key));
        markIteratorDirty();
        if (root == nullptr || comp(root->pair.key, key) || comp(key, root->pair.key))
        {
            return endIt;
        }
        return iterator(root);
    }
    iterator erase(const key_type& key)
    {
        root = remove(root, key);
        markIteratorDirty();
        return iterator(root);	
    }
    iterator erase(K&& key)
    {
        root = remove(root, std::move(key));
        markIteratorDirty();
        return iterator(root);
    }
    void clear()
    {
        nodeContainer.clear();
        root = nullptr;
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
        Node *beginNode = root;
        if (root == nullptr)
        {
            return Iterator(nullptr);
        }
        else
        {
            Array<Node *> beginTraversal;
            while (beginNode != nullptr)
            {
                beginTraversal.add(beginNode);
                beginNode = beginNode->leftChild;
            }
            beginNode = beginTraversal.back();
            beginTraversal.pop();
            return Iterator(beginNode, std::move(beginTraversal));
        }
    }
    inline Iterator calcEndIterator() const
    {
        Node *endNode = root;
        if (root == nullptr)
        {
            return Iterator(nullptr);
        }
        else
        {
            Array<Node *> endTraversal;
            while (endNode != nullptr)
            {
                endTraversal.add(endNode);
                endNode = endNode->rightChild;
            }
            return Iterator(endNode, std::move(endTraversal));
        }
    }
    Array<Node, NodeAlloc> nodeContainer;
    Node *root;
    Iterator beginIt;
    Iterator endIt;
    bool iteratorsDirty;
    uint32 _size;
    Compare comp;
    Node *rotateRight(Node *node)
    {
        Node *y = node->leftChild;
        node->leftChild = y->rightChild;
        y->rightChild = node;
        return y;
    }
    Node *rotateLeft(Node *node)
    {
        Node *y = node->rightChild;
        node->rightChild = y->leftChild;
        y->leftChild = node;
        return y;
    }
    template<class KeyType>
    Node *insert(Node *r, KeyType&& key)
    {
        if (r == nullptr)
        {
            return &nodeContainer.emplace(std::forward<KeyType>(key));
        }
        r = splay(r, key);

        if (!(comp(r->pair.key, key) || comp(key, r->pair.key)))
            return r;

        Node *newNode = &nodeContainer.emplace(std::forward<KeyType>(key));

        if (comp(key, r->pair.key))
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
    Node *remove(Node *r, KeyType&& key)
    {
        Node *temp;
        if (!r)
            return nullptr;

        r = splay(r, key);

        if (comp(r->pair.key, key) || comp(key, r->pair.key))
            return r;

        if (!r->leftChild)
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
        Node& lastNode = nodeContainer.back();
        size_t removedIndex = nodeContainer.indexOf(temp);
        nodeContainer[removedIndex] = std::move(lastNode);
        for(auto it : nodeContainer)
        {
            if(it.leftChild == &lastNode)
            {
                it.leftChild = &nodeContainer[removedIndex];
            }
            if(it.rightChild == &lastNode)
            {
                it.rightChild = &nodeContainer[removedIndex];
            }
        }
        nodeContainer.pop();
        _size--;
        return r;
    }
    template<class KeyType>
    Node *splay(Node *r, KeyType&& key)
    {
        if (r == nullptr || !(comp(r->pair.key, key) || comp(key, r->pair.key)))
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
};
} // namespace Seele