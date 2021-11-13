#pragma once
#include <memory_resource>

namespace Seele
{
template <typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
class List
{
private:
    struct Node
    {
        Node *prev;
        Node *next;
        T data;
    };
    using NodeAllocator = std::allocator_traits<Allocator>::template rebind_alloc<Node>;

public:
    template <typename X>
    class IteratorBase
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = X;
        using difference_type = std::ptrdiff_t;
        using reference = X&;
        using pointer = X*;

        IteratorBase(Node *x = nullptr)
            : node(x)
        {
        }
        IteratorBase(const IteratorBase &i)
            : node(i.node)
        {
        }
        IteratorBase(IteratorBase&& i)
            : node(std::move(i.node))
        {
        }
        ~IteratorBase()
        {
        }
        IteratorBase& operator=(const IteratorBase& other)
        {
            if(this != &other)
            {
                node = other.node;
            }
            return *this;
        }
        IteratorBase& operator=(IteratorBase&& other)
        {
            if(this != &other)
            {
                node = std::move(other.node);
            }
            return *this;
        }
        reference operator*() const
        {
            return node->data;
        }
        pointer operator->() const
        {
            return &node->data;
        }
        inline bool operator!=(const IteratorBase &other)
        {
            return node != other.node;
        }
        inline bool operator==(const IteratorBase &other)
        {
            return node == other.node;
        }
        IteratorBase &operator--()
        {
            node = node->prev;
            return *this;
        }
        IteratorBase operator--(int)
        {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }
        IteratorBase &operator++()
        {
            node = node->next;
            return *this;
        }
        IteratorBase operator++(int)
        {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }

    private:
        Node *node;
        friend class List<T>;
    };
    
    using Iterator = IteratorBase<T>;
    using ConstIterator = IteratorBase<const T>;
    
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = T*;
    using const_pointer = const T*;

    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    List()
        : root(nullptr)
        , tail(nullptr)
        , beginIt(Iterator(root))
        , endIt(Iterator(tail))
        , _size(0)
        , allocator(NodeAllocator())
    {
    }
    List(const List& other)
    {
        //TODO: improve
        for(const auto& it : other)
        {
            add(it);
        }
    }
    List(List&& other)
        : root(std::move(other.root))
        , tail(std::move(other.tail))
        , beginIt(std::move(other.beginIt))
        , endIt(std::move(other.endIt))
        , _size(std::move(other._size))
    {
        other._size = 0;
    }
    ~List()
    {
        clear();
    }
    List& operator=(const List& other)
    {
        if(this != &other)
        {
            if(root != nullptr)
            {
                delete root;
            }
            if(tail != nullptr)
            {
                delete tail;
            }
            _size = 0;
            for(const auto& it : other)
            {
                add(it);
            }
            markIteratorDirty();
        }
        return *this;
    }
    List& operator=(List&& other)
    {
        if(this != &other)
        {
            if(root != nullptr)
            {
                clear();
            }
            root = other.root;
            tail = other.tail;
            beginIt = other.beginIt;
            endIt = other.endIt;
            _size = other._size;
            other._size = 0;
            markIteratorDirty();
        }
        return *this;
    }
    
    reference front()
    {
        return root->data;
    }
    reference back()
    {
        return tail->prev->data;
    }
    void clear()
    {
        if (empty())
        {
            return;
        }
        for (Node *tmp = root; tmp != tail;)
        {
            tmp = tmp->next;
            deallocateNode(tmp->prev);
        }
        deallocateNode(tail);
        markIteratorDirty();
        tail = nullptr;
        root = nullptr;
    }
    //Insert at the end
    iterator add(const T &value)
    {
        if (root == nullptr)
        {
            root = allocateNode();
            tail = root;
        }
        initializeNode(tail, value);
        Node *newTail = allocateNode();
        newTail->prev = tail;
        newTail->next = nullptr;
        tail->next = newTail;
        iterator insertedElement(tail);
        tail = newTail;
        markIteratorDirty();
        _size++;
        return insertedElement;
    }
    iterator add(T&& value)
    {
        if (root == nullptr)
        {
            root = allocateNode();
            tail = root;
        }
        initializeNode(tail, std::move(value));
        Node *newTail = allocateNode();
        newTail->prev = tail;
        newTail->next = nullptr;
        tail->next = newTail;
        Iterator insertedElement(tail);
        tail = newTail;
        markIteratorDirty();
        _size++;
        return insertedElement;
    }
    // takes all elements from other and move-inserts them into
    // this, clearing other in the process
    void moveElements(List& other){
        tail->prev->next = other.root;
        other.root->prev = tail->prev;
        _size += other._size;
        deallocateNode(tail);
        tail = other.tail;
        other._size = 0;
        other.root = nullptr;
        other.tail = nullptr;
        markIteratorDirty();
        other.markIteratorDirty();
    }
    template<typename... args>
    constexpr reference emplace(args... arguments)
    {
        if (root == nullptr)
        {
            root = allocateNode();
            tail = root;
        }
        std::allocator_traits<NodeAllocator>::construct(allocator, 
            tail, 
            tail->prev, 
            tail->next, 
            arguments...);
        Node *newTail = allocateNode();
        newTail->prev = tail;
        newTail->next = nullptr;
        tail->next = newTail;
        Iterator insertedElement(tail);
        tail = newTail;
        markIteratorDirty();
        _size++;
        return insertedElement;
    }
    // front + popFront
    value_type&& retrieve()
    {
        auto&& temp = std::move(root->data);
        popFront();
        return std::move(temp);
    }
    iterator remove(iterator pos)
    {
        _size--;
        Node *prev = pos.node->prev;
        Node *next = pos.node->next;
        if (prev == nullptr)
        {
            root = next;
        }
        else
        {
            prev->next = next;
        }
        if(next == nullptr)
        {
            root = prev;
        }
        else
        {
            next->prev = prev;
        }
        deallocateNode(pos.node);
        markIteratorDirty();
        return Iterator(next);
    }
    void popBack()
    {
        assert(_size > 0);
        remove(Iterator(tail->prev));
    }
    void popFront()
    {
        assert(_size > 0);
        remove(Iterator(root));
    }
    iterator insert(iterator pos, const T &value)
    {
        _size++;
        if (root == nullptr)
        {
            root = allocateNode();
            root->data = value;
            tail = allocateNode();
            root->next = tail;
            root->prev = nullptr;
            tail->prev = root;
            tail->next = nullptr;
            markIteratorDirty();
            return beginIt;
        }
        Node *tmp = pos.node->prev;
        Node *newNode = allocateNode();
        initializeNode(newNode, value);
        tmp->next = newNode;
        newNode->prev = tmp;
        newNode->next = pos.node;
        pos.node->prev = newNode;
        markIteratorDirty();
        return Iterator(newNode);
    }
    iterator find(const T &value)
    {
        for (Node *i = root; i != tail; i = i->next)
        {
            if (!(i->data < value) && !(value < i->data))
            {
                return iterator(i);
            }
        }
        return endIt;
    }
    bool empty() const
    {
        return _size == 0;
    }
    size_type size() const
    {
        return _size;
    }
    iterator begin()
    {
        return beginIt;
    }
    const_iterator begin() const
    {
        return cbeginIt;
    }
    iterator end()
    {
        return endIt;
    }
    const_iterator end() const
    {
        return cendIt;
    }

private:
    Node* allocateNode()
    {
        Node* node = allocator.allocate(1);
        assert(node != nullptr);
        node->prev = nullptr;
        node->next = nullptr;
        return node;
    }
    template<typename Type>
    void initializeNode(Node* node, Type&& data)
    {
        std::allocator_traits<NodeAllocator>::construct(allocator, 
            node, 
            node->prev, 
            node->next, 
            std::forward<Type>(data));
    }
    void deallocateNode(Node* node)
    {
        allocator.deallocate(node, 1);
    }
    void markIteratorDirty()
    {
        beginIt = Iterator(root);
        endIt = Iterator(tail);
        cbeginIt = ConstIterator(root);
        cendIt = ConstIterator(tail);
    }
    Node *root;
    Node *tail;
    Iterator beginIt;
    Iterator endIt;
    ConstIterator cbeginIt;
    ConstIterator cendIt;
    size_type _size;
    NodeAllocator allocator;
};
} // namespace Seele