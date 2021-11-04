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
        size_t self;
        size_t leftChild;
        size_t rightChild;
        Pair<K, V> pair;
        Node()
            : self(-1)
            , leftChild(-1)
            , rightChild(-1)
            , pair()
        {
        }
        Node(const Node& other) = default;
        Node(Node&& other) = default;
        Node& operator=(const Node& other) = default;
        Node& operator=(Node&& other) = default;
        Node(size_t self, K key)
            : self(self)
            , leftChild(-1)
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

        IteratorBase(Node *x = nullptr)
            : node(x)
        {
        }
        IteratorBase(Node *x, Array<size_t> &&beginIt, const Array<Node, NodeAlloc>* nodeContainer)
            : node(x), traversal(std::move(beginIt)), nodeContainer(nodeContainer)
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
            size_t nextIndex = node->rightChild;
            while (nextIndex != -1 && (*nodeContainer)[nextIndex].leftChild != -1)
            {
                traversal.add(nextIndex);
                nextIndex = (*nodeContainer)[nextIndex].leftChild;
            }
            if (nextIndex == -1 && traversal.size() > 0)
            {
                nextIndex = traversal.back();
                traversal.pop();
            }
            node = nextIndex != -1 ? &(*nodeContainer)[nextIndex] : nullptr;
            return *this;
        }
        IteratorBase &operator--()
        {
            size_t nextIndex = node->leftChild;
            while (nextIndex != -1 && (*nodeContainer)[nextIndex].rightChild != -1)
            {
                traversal.add(nextIndex);
                nextIndex = (*nodeContainer)[nextIndex].rightchild;
            }
            if (nextIndex == -1 && traversal.size() > 0)
            {
                nextIndex = traversal.back();
                traversal.pop();
            }
            node = nextIndex != -1 ? &(*nodeContainer)[nextIndex] : nullptr;
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
        refreshIterators();
        if (root == nullptr || comp(root->pair.key, key) || comp(key, root->pair.key))
        {
            return endIt;
        }
        return iterator(root);
    }
    iterator find(key_type&& key)
    {
        root = splay(root, std::move(key));
        refreshIterators();
        if (root == nullptr || comp(root->pair.key, key) || comp(key, root->pair.key))
        {
            return endIt;
        }
        return iterator(root);
    }
    iterator erase(const key_type& key)
    {
        root = remove(root, key);
        refreshIterators();
        return iterator(root);	
    }
    iterator erase(K&& key)
    {
        root = remove(root, std::move(key));
        refreshIterators();
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
    Node* leftChild(Node* node)
    {
        if(node->leftChild >= nodeContainer.size()) return nullptr;
        return &nodeContainer[node->leftChild];
    }
    Node* rightChild(Node* node)
    {
        if(node->rightChild >= nodeContainer.size()) return nullptr;
        return &nodeContainer[node->rightChild];
    }
    Node* leftChild(Node* node) const
    {
        if(node->leftChild >= nodeContainer.size()) return nullptr;
        return &nodeContainer[node->leftChild];
    }
    Node* rightChild(Node* node) const
    {
        if(node->rightChild >= nodeContainer.size()) return nullptr;
        return &nodeContainer[node->rightChild];
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
        if (root == nullptr)
        {
            return Iterator(nullptr);
        }
        else
        {
            size_t beginIndex = root->self;
            Array<size_t> beginTraversal;
            while (beginIndex < nodeContainer.size())
            {
                beginTraversal.add(beginIndex);
                beginIndex = nodeContainer[beginIndex].leftChild;
            }
            Node* beginNode = &nodeContainer[beginTraversal.back()];
            beginTraversal.pop();
            return Iterator(beginNode, std::move(beginTraversal), &nodeContainer);
        }
    }
    inline Iterator calcEndIterator() const
    {
        if (root == nullptr)
        {
            return Iterator(nullptr);
        }
        else
        {
            size_t endIndex = root->self;
            Array<size_t> endTraversal;
            while (endIndex < nodeContainer.size())
            {
                endTraversal.add(endIndex);
                endIndex = nodeContainer[endIndex].rightChild;
            }
            return Iterator(nullptr, std::move(endTraversal), &nodeContainer);
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
        Node *y = leftChild(node);
        node->leftChild = y->rightChild;
        y->rightChild = node->self;
        return y;
    }
    Node *rotateLeft(Node *node)
    {
        Node *y = rightChild(node);
        node->rightChild = y->leftChild;
        y->leftChild = node->self;
        return y;
    }
    template<class KeyType>
    Node *insert(Node *r, KeyType&& key)
    {
        if (r == nullptr)
        {
            return &nodeContainer.emplace(nodeContainer.size(), std::forward<KeyType>(key));
        }
        r = splay(r, key);

        if (!(comp(r->pair.key, key) || comp(key, r->pair.key)))
            return r;

        Node *newNode = &nodeContainer.emplace(nodeContainer.size(), std::forward<KeyType>(key));

        if (comp(key, r->pair.key))
        {
            newNode->rightChild = r->self;
            newNode->leftChild = r->leftChild;
            r->leftChild = -1;
        }
        else
        {
            newNode->leftChild = r->self;
            newNode->rightChild = r->rightChild;
            r->rightChild = -1;
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

        if (r->leftChild == -1)
        {
            temp = r;
            r = rightChild(r);
        }
        else
        {
            temp = r;

            r = splay(leftChild(r), key);
            r->rightChild = temp->rightChild;
        }
        Node& lastNode = nodeContainer.back();
        size_t removedIndex = temp->self;
        //Arrays can only pop back, so we need to move the last element to the deleted index
        if(removedIndex != lastNode.self)
        {
            nodeContainer[removedIndex] = std::move(lastNode);
            for(auto it : nodeContainer)
            {
                if(it.leftChild == lastNode.self)
                {
                    it.leftChild = removedIndex;
                }
                if(it.rightChild == lastNode.self)
                {
                    it.rightChild = removedIndex;
                }
            }
            lastNode.self = removedIndex;
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
            if (r->leftChild >= nodeContainer.size())
                return r;

            if (comp(key, leftChild(r)->pair.key))
            {
                Node* res = splay(leftChild(leftChild(r)), key);
                leftChild(r)->leftChild = res ? res->self : -1;

                r = rotateRight(r);
            }
            else if (comp(leftChild(r)->pair.key, key))
            {
                Node* res = splay(rightChild(leftChild(r)), key);
                leftChild(r)->rightChild = res ? res->self : -1;

                if (leftChild(r)->rightChild < nodeContainer.size())
                {
                    r->leftChild = rotateLeft(leftChild(r))->self;
                }
            }
            return (r->leftChild >= nodeContainer.size()) ? r : rotateRight(r);
        }
        else
        {
            if (r->rightChild >= nodeContainer.size())
                return r;

            if (comp(key, rightChild(r)->pair.key))
            {
                Node* res = splay(leftChild(rightChild(r)), key);
                rightChild(r)->leftChild = res ? res->self : -1;

                if (rightChild(r)->leftChild < nodeContainer.size())
                {
                    r->rightChild = rotateRight(rightChild(r))->self;
                }
            }
            else if (comp(rightChild(r)->pair.key, key))
            {
                Node* res = splay(rightChild(rightChild(r)), key);
                rightChild(r)->rightChild = res ? res->self : -1;
                r = rotateLeft(r);
            }
            return (r->rightChild >= nodeContainer.size()) ? r : rotateLeft(r);
        }
    }
};
} // namespace Seele