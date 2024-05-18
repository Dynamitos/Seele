#pragma once
#include <memory_resource>
#include <assert.h>

namespace Seele
{
template <typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
class List
{
private:
    struct Node
    {
        Node(Node* prev, Node* next, const T& data)
            : prev(prev)
            , next(next)
            , data(data)
        {}
        Node(Node* prev, Node* next, T&& data)
            : prev(prev)
            , next(next)
            , data(std::move(data))
        {}
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
        IteratorBase(IteratorBase&& i) noexcept
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
        IteratorBase& operator=(IteratorBase&& other) noexcept
        {
            if(this != &other)
            {
                node = std::move(other.node);
            }
            return *this;
        }
        constexpr reference operator*() const
        {
            return node->data;
        }
        constexpr pointer operator->() const
        {
            return &node->data;
        }
        constexpr bool operator!=(const IteratorBase &other)
        {
            return node != other.node;
        }
        constexpr bool operator==(const IteratorBase &other)
        {
            return node == other.node;
        }
        constexpr std::strong_ordering operator<=>(const IteratorBase& other)
        {
            return node <=> other.node;
        }
        constexpr IteratorBase &operator--()
        {
            node = node->prev;
            return *this;
        }
        constexpr IteratorBase operator--(int)
        {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }
        constexpr IteratorBase &operator++()
        {
            node = node->next;
            return *this;
        }
        constexpr IteratorBase operator++(int)
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
    
    constexpr List()
        : allocator(Allocator())
        , root(allocateNode())
        , tail(root)
        , beginIt(Iterator(root))
        , endIt(Iterator(tail))
        , _size(0)
    {
    }
    constexpr explicit List(const Allocator& alloc)
        : allocator(alloc)
        , root(allocateNode())
        , tail(root)
        , beginIt(Iterator(root))
        , endIt(Iterator(tail))
        , _size(0)
    {
    }
    constexpr List(size_type count, const T& value = T(), const Allocator& alloc = Allocator())
        : List(alloc)
    {
        for(size_type i = 0; i < count; ++i)
        {
            add(value);
        }
    }
    constexpr List(size_type count, const Allocator& alloc = Allocator())
        : List(alloc)
    {
        for(size_type i = 0; i < count; ++i)
        {
            add(T());
        }
    }
    constexpr List(const List& other)
        : List(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.allocator))
    {
        //TODO: improve
        for(const auto& it : other)
        {
            add(it);
        }
    }
    
    constexpr List(const List& other, const Allocator& alloc)
        : List(alloc)
    {
        //TODO: improve
        for(const auto& it : other)
        {
            add(it);
        }
    }
    constexpr List(List&& other)
        : allocator(std::move(other.allocator))
        , root(std::move(other.root))
        , tail(std::move(other.tail))
        , beginIt(std::move(other.beginIt))
        , endIt(std::move(other.endIt))
        , _size(std::move(other._size))
    {
        other.root = nullptr;
        other.tail = nullptr;
        other._size = 0;
    }
    constexpr List(List&& other, const Allocator& alloc)
        : allocator(alloc)
        , root(std::move(other.root))
        , tail(std::move(other.tail))
        , beginIt(std::move(other.beginIt))
        , endIt(std::move(other.endIt))
        , _size(std::move(other._size))
    {
        other.root = nullptr;
        other.tail = nullptr;
        other._size = 0;
    }
    constexpr ~List()
    {
        clear();
        deallocateNode(tail);
    }
    constexpr List& operator=(const List& other)
    {
        if(this != &other)
        {
            clear();
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value )
            {
                if (!std::allocator_traits<allocator_type>::is_always_equal::value
                    && allocator != other.allocator)
                {
                    clear();
                }
                allocator = other.allocator;
            }
            for(const auto& it : other)
            {
                add(it);
            }
            markIteratorDirty();
        }
        return *this;
    }
    constexpr List& operator=(List&& other)
    {
        if(this != &other)
        {
            clear();
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
            {
                allocator = std::move(other.allocator);
            }
            root = other.root;
            tail = other.tail;
            beginIt = other.beginIt;
            endIt = other.endIt;
            _size = other._size;
            other.root = nullptr;
            other.tail = nullptr;
            other._size = 0;
            markIteratorDirty();
        }
        return *this;
    }
    
    constexpr reference front()
    {
        return root->data;
    }
    constexpr reference back()
    {
        return tail->prev->data;
    }
    constexpr void clear()
    {
        if (empty())
        {
            return;
        }
        for (Node *tmp = root; tmp != tail;)
        {
            tmp = tmp->next;
            destroyNode(tmp->prev);
            deallocateNode(tmp->prev);
        }
        root = tail;
        markIteratorDirty();
        _size = 0;
    }
    //Insert at the end
    constexpr iterator add(const T &value)
    {
        return addInternal(value);
    }
    constexpr iterator add(T&& value)
    {
        return addInternal(std::move(value));
    }
    template<typename... args>
    constexpr reference emplace(args... arguments)
    {
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
    constexpr value_type retrieve()
    {
        value_type temp = std::move(root->data);
        popFront();
        return temp;
    }
    constexpr iterator remove(iterator pos)
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
            tail = prev;
        }
        else
        {
            next->prev = prev;
        }
        destroyNode(pos.node);
        deallocateNode(pos.node);
        markIteratorDirty();
        return Iterator(next);
    }
    constexpr void popBack()
    {
        assert(_size > 0);
        remove(Iterator(tail->prev));
    }
    constexpr void pop_back()
    {
        assert(_size > 0);
        remove(Iterator(tail->prev));
    }
    constexpr void popFront()
    {
        assert(_size > 0);
        remove(Iterator(root));
    }
    constexpr void pop_front()
    {
        assert(_size > 0);
        remove(Iterator(root));
    }
    constexpr iterator insert(iterator pos, const T &value)
    {
        _size++;
        Node *newNode = allocateNode();
        initializeNode(newNode, value);
        newNode->next = pos.node;
        pos.node->prev = newNode;
        Node *tmp = pos.node->prev;
        if (tmp != nullptr)
        {
            tmp->next = newNode;
            newNode->prev = tmp;
        }
        else
        {
            root = newNode;
        }
        markIteratorDirty();
        return Iterator(newNode);
    }
    constexpr iterator find(const T &value)
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
    constexpr bool empty() const
    {
        return _size == 0;
    }
    constexpr size_type size() const
    {
        return _size;
    }
    constexpr iterator begin()
    {
        return beginIt;
    }
    constexpr const_iterator begin() const
    {
        return cbeginIt;
    }
    constexpr iterator end()
    {
        return endIt;
    }
    constexpr const_iterator end() const
    {
        return cendIt;
    }

private:
    constexpr Node* allocateNode()
    {
        Node* node = allocator.allocate(1);
        assert(node != nullptr);
        node->prev = nullptr;
        node->next = nullptr;
        return node;
    }
    template<typename Type>
    constexpr void initializeNode(Node* node, Type&& data)
    {
        std::allocator_traits<NodeAllocator>::construct(allocator, 
            node, 
            node->prev, 
            node->next, 
            std::forward<Type>(data));
    }
    constexpr void destroyNode(Node* node)
    {
        std::allocator_traits<NodeAllocator>::destroy(allocator, node);
    }
    constexpr void deallocateNode(Node* node)
    {
        allocator.deallocate(node, 1);
    }
    template<typename ValueType>
    constexpr iterator addInternal(ValueType&& value)
    {
        initializeNode(tail, std::forward<ValueType>(value));
        Node* newTail = allocateNode();
        newTail->prev = tail;
        newTail->next = nullptr;
        tail->next = newTail;
        Iterator insertedElement(tail);
        tail = newTail;
        markIteratorDirty();
        _size++;
        return insertedElement;
    }
    constexpr void markIteratorDirty()
    {
        beginIt = Iterator(root);
        endIt = Iterator(tail);
        cbeginIt = ConstIterator(root);
        cendIt = ConstIterator(tail);
    }
    NodeAllocator allocator;
    Node *root;
    Node *tail;
    iterator beginIt;
    iterator endIt;
    const_iterator cbeginIt;
    const_iterator cendIt;
    size_type _size;
};
} // namespace Seele