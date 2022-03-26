#pragma once
#include "MinimalEngine.h"
#include "Array.h"

namespace Seele
{
template<typename T, Allocator = std::pmr::polymorphic_allocator<T>>
class RingBuffer
{
public:
    template <typename X>
    class IteratorBase
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = X;
        using difference_type = std::ptrdiff_t;
        using reference = X&;
        using pointer = X*;

        IteratorBase(Array<T>& arr, size_t index)
            : arr(arr)
            , index(index)
        {
        }
        reference operator*() const
        {
            return arr[index];
        }
        pointer operator->() const
        {
            return &arr[index];
        }
        inline bool operator!=(const IteratorBase &other) const
        {
            return arr != other.arr && index != other.index;
        }
        inline bool operator==(const IteratorBase &other) const
        {
            return arr == other.arr && index == other.index;
        }
        inline int operator-(const IteratorBase &other) const
        {
            return (int)(index - other.index);
        }
        IteratorBase &operator++()
        {
            index = (index + 1) % arr.size();
            return *this;
        }
        IteratorBase &operator--()
        {
            index = (index + 1) % arr.size();
            return *this;
        }
        IteratorBase operator++(int)
        {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }
        IteratorBase operator--(int)
        {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }

    private:
        Array<T, Allocator>& arr;
        size_t index;
    };
    
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = value_type&;
    using const_reference = const value_type&;

    using Iterator = IteratorBase<T>;
    using ConstIterator = IteratorBase<const T>;
    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr RingBuffer() noexcept(noexcept(Allocator()))
    {
        refreshIterators();
    }

    constexpr explicit RingBuffer(const allocator_type& alloc)
        : data(alloc)
    {
        refreshIterators();
    }
    constexpr RingBuffer(size_type size, const value_type& value, const allocator_type& alloc = allocator_type())
        : data(size, value, alloc)
        , end(size)
    {
        refreshIterators();
    }
    constexpr explicit RingBuffer(size_type size, const allocator_type& alloc = allocator_type())
        : arraySize(size)
        , allocated(size)
        , allocator(alloc)
    {
        _data = allocateRingBuffer(size);
        assert(_data != nullptr);
        for (size_type i = 0; i < size; ++i)
        {
            std::allocator_traits<allocator_type>::construct(allocator, &_data[i]);
        }
        markIteratorDirty();
    }
    constexpr RingBuffer(std::initializer_list<T> init, const allocator_type& alloc = allocator_type())
        : arraySize(init.size())
        , allocated(init.size())
        , allocator(alloc)
    {
        _data = allocateRingBuffer(init.size());
        assert(_data != nullptr);
        markIteratorDirty();
        std::uninitialized_copy(init.begin(), init.end(), begin());
    }
    RingBuffer(const RingBuffer &other)
        : arraySize(other.arraySize)
        , allocated(other.allocated)
        , allocator(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.allocator))
    {
        _data = allocateRingBuffer(other.allocated);
        assert(_data != nullptr);
        markIteratorDirty();
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }
    RingBuffer(RingBuffer &&other) noexcept
        : arraySize(std::move(other.arraySize))
        , allocated(std::move(other.allocated))
        , allocator(std::move(other.allocator))
    {
        _data = other._data;
        other._data = nullptr;
        other.allocated = 0;
        other.arraySize = 0;
        markIteratorDirty();
    }
    RingBuffer &operator=(const RingBuffer &other) noexcept
    {
        if (this != &other)
        {
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value )
            {
                if (!std::allocator_traits<allocator_type>::is_always_equal::value
                    && allocator != other.allocator)
                {
                    clear();
                }
                allocator = other.allocator;
            }
            if (other.arraySize > allocated)
            {
                clear();
            }
            if(_data == nullptr)
            {
                _data = allocateRingBuffer(other.allocated);
                allocated = other.allocated;
            }
            arraySize = other.arraySize;
            markIteratorDirty();
            std::uninitialized_copy(other.begin(), other.end(), begin());
        }
        return *this;
    }
    RingBuffer &operator=(RingBuffer &&other) noexcept
    {
        if (this != &other)
        {
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
            {
                allocator = std::move(other.allocator);
            }
            if (_data != nullptr)
            {
                clear();
            }
            allocated = std::move(other.allocated);
            arraySize = std::move(other.arraySize);
            _data = other._data;
            other._data = nullptr;
            markIteratorDirty();
        }
        return *this;
    }
    ~RingBuffer()
    {
        clear();
    }

    constexpr bool operator==(const RingBuffer &other)
    {
        return _data == other._data;
    }

    constexpr bool operator!=(const RingBuffer &other)
    {
        return !(*this == other);
    }

    constexpr iterator find(const value_type &item)
    {
        for (uint32 i = 0; i < arraySize; ++i)
        {
            if (_data[i] == item)
            {
                return iterator(&_data[i]);
            }
        }
        return endIt;
    }
    constexpr iterator find(value_type&& item)
    {
        for (uint32 i = 0; i < arraySize; ++i)
        {
            if (_data[i] == item)
            {
                return iterator(&_data[i]);
            }
        }
        return endIt;	
    }
    constexpr allocator_type get_allocator() const
    {
        return allocator;
    }
    constexpr iterator begin() const
    {
        return beginIt;
    }
    constexpr iterator end() const
    {
        return endIt;
    }
    constexpr const_iterator cbegin() const
    {
        return beginIt;
    }
    constexpr const_iterator cend() const
    {
        return endIt;
    }
    constexpr reference add(const value_type &item = value_type())
    {
        return addInternal(item);
    }
    constexpr reference add(value_type&& item)
    {
        return addInternal(std::forward<T>(item));
    }
    constexpr void addAll(RingBuffer other)
    {
        for(auto value : other)
        {
            addInternal(value);
        }
    }
    constexpr reference addUnique(const value_type &item = value_type())
    {
        iterator it;
        if((it = std::move(find(item))) != endIt)
        {
            return *it;
        }
        return addInternal(item);
    }
    template<typename... args>
    constexpr reference emplace(args... arguments)
    {
        if (arraySize == allocated)
        {
            size_type newSize = arraySize + 1;
            allocated = calculateGrowth(newSize);
            T *tempRingBuffer = allocateRingBuffer(allocated);
            assert(tempRingBuffer != nullptr);
            
            std::uninitialized_move(begin(), end(), Iterator(tempRingBuffer));
            deallocateRingBuffer(_data, arraySize);
            _data = tempRingBuffer;
        }
        std::allocator_traits<allocator_type>::construct(allocator, &_data[arraySize++], arguments...);
        markIteratorDirty();
        return _data[arraySize - 1];
    }
    constexpr void remove(iterator it, bool keepOrder = true)
    {
        remove(it - beginIt, keepOrder);
    }
    constexpr void remove(size_type index, bool keepOrder = true)
    {
        if (keepOrder)
        {
            for(uint32 i = index; i < arraySize-1; ++i)
            {
                _data[i] = std::move(_data[i+1]);
            }
        }
        else
        {
            _data[index] = std::move(_data[arraySize - 1]);
        }
        std::allocator_traits<allocator_type>::destroy(allocator, &_data[--arraySize]);
        markIteratorDirty();
    }
    constexpr void resize(size_type newSize)
    {
        resizeInternal(newSize, std::move(T()));
    }
    constexpr void resize(size_type newSize, const value_type& value)
    {
        resizeInternal(newSize, value);
    }
    constexpr void clear()
    {
        if(_data == nullptr)
        {
            return;
        }
        for(size_type i = 0; i < arraySize; ++i)
        {
            std::allocator_traits<allocator_type>::destroy(allocator, &_data[i]);
        }
        deallocateRingBuffer(_data, allocated);
        _data = nullptr;
        arraySize = 0;
        allocated = 0;
        markIteratorDirty();
    }
    inline size_type indexOf(iterator iterator)
    {
        return iterator - beginIt;
    }
    inline size_type indexOf(const_iterator iterator) const
    {
        return iterator.p - beginIt.p;
    }
    inline size_type indexOf(T& t)
    {
        return indexOf(find(t));
    }
    inline size_type indexOf(const T& t) const
    {
        return indexOf(find(t));
    }
    inline size_type size() const
    {
        return arraySize;
    }
    inline size_type empty() const
    {
        return arraySize == 0;
    }
    inline size_type capacity() const
    {
        return allocated;
    }
    inline pointer data() const
    {
        return _data;
    }
    inline reference front() const
    {
        assert(arraySize > 0);
        return _data[0];
    }
    inline reference back() const
    {
        assert(arraySize > 0);
        return _data[arraySize - 1];
    }
    void pop()
    {
        std::allocator_traits<allocator_type>::destroy(allocator, &_data[--arraySize]);
        markIteratorDirty();
    }
    constexpr inline reference operator[](size_type index)
    {
        assert(index < arraySize);
        return _data[index];
    }
    constexpr inline const reference operator[](size_type index) const
    {
        assert(index < arraySize);
        return _data[index];
    }
private:
    void refreshIterators()
    {
        beginIt = Iterator(data, begin);
        endIt = Iterator(data, end);
    }
    size_type begin = 0;
    size_type end = 0;
    iterator beginIt;
    iterator endIt;
    Array<T, Allocator> data;
};
class StaticRingBuffer
{

};
} // namespace Seele
