#pragma once
#include "EngineTypes.h"
#include <initializer_list>
#include <iterator>
#include <assert.h>
#include <memory_resource>
#include <algorithm>

#ifndef DEFAULT_ALLOC_SIZE
#define DEFAULT_ALLOC_SIZE 16
#endif

namespace Seele
{
template <typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
struct Array
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

        constexpr IteratorBase(X *x = nullptr)
            : p(x)
        {
        }
        constexpr reference operator*() const
        {
            return *p;
        }
        constexpr pointer operator->() const
        {
            return p;
        }
        constexpr bool operator!=(const IteratorBase &other) const
        {
            return p != other.p;
        }
        constexpr bool operator==(const IteratorBase &other) const
        {
            return p == other.p;
        }
        constexpr IteratorBase operator+(size_t other) const
        {
            IteratorBase tmp(*this);
            tmp.p += other;
            return tmp;
        }
        constexpr int operator-(const IteratorBase &other) const
        {
            return (int)(p - other.p);
        }
        constexpr IteratorBase& operator+=(difference_type other)
        {
            p+=other;
            return *this;
        }
        constexpr bool operator<(const IteratorBase& other) const
        {
            return p < other.p;
        }
        constexpr IteratorBase operator-(difference_type diff) const
        {
            return IteratorBase(p - diff);
        }
        constexpr IteratorBase &operator++()
        {
            p++;
            return *this;
        }
        constexpr IteratorBase &operator--()
        {
            p--;
            return *this;
        }
        constexpr IteratorBase operator++(int)
        {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }
        constexpr IteratorBase operator--(int)
        {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }
    private:
        X *p;
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

    constexpr Array() noexcept(noexcept(Allocator()))
        : arraySize(0)
        , allocated(DEFAULT_ALLOC_SIZE)
        , allocator(Allocator())
    {
        _data = allocateArray(DEFAULT_ALLOC_SIZE);
        assert(_data != nullptr);
    }

    constexpr explicit Array(const allocator_type& alloc) noexcept
        : arraySize(0)
        , allocated(DEFAULT_ALLOC_SIZE)
        , allocator(alloc)
    {
        _data = allocateArray(DEFAULT_ALLOC_SIZE);
        assert(_data != nullptr);
    }
    constexpr Array(size_type size, const value_type& value, const allocator_type& alloc = allocator_type())
        : arraySize(size)
        , allocated(size)
        , allocator(alloc)
    {
        _data = allocateArray(size);
        assert(_data != nullptr);
        for (size_type i = 0; i < size; ++i)
        {
            std::allocator_traits<allocator_type>::construct(allocator, &_data[i], value);
        }
    }
    constexpr explicit Array(size_type size, const allocator_type& alloc = allocator_type())
        : arraySize(size)
        , allocated(size)
        , allocator(alloc)
    {
        _data = allocateArray(size);
        assert(_data != nullptr);
        if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>)
        {
            std::memset(_data, 0, size * sizeof(T));
        }
        else
        {
            for (size_type i = 0; i < size; ++i)
            {
                std::allocator_traits<allocator_type>::construct(allocator, &_data[i]);
            }
        }
    }
    constexpr Array(std::initializer_list<T> init, const allocator_type& alloc = allocator_type())
        : arraySize(init.size())
        , allocated(init.size())
        , allocator(alloc)
    {
        _data = allocateArray(init.size());
        assert(_data != nullptr);
        std::uninitialized_copy(init.begin(), init.end(), begin());
    }
    
    constexpr Array(const Array &other)
        : arraySize(other.arraySize)
        , allocated(other.allocated)
        , allocator(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.allocator))
    {
        _data = allocateArray(other.allocated);
        assert(_data != nullptr);
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }
    constexpr Array(const Array& other, const Allocator& alloc)
        : arraySize(other.arraySize)
        , allocated(other.allocated)
        , allocator(alloc)
    {
        _data = allocateArray(other.allocated);
        assert(_data != nullptr);
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }
    constexpr Array(Array &&other) noexcept
        : arraySize(std::move(other.arraySize))
        , allocated(std::move(other.allocated))
        , allocator(std::move(other.allocator))
    {
        _data = other._data;
        other._data = nullptr;
        other.allocated = 0;
        other.arraySize = 0;
    }
    constexpr Array(Array &&other, const Allocator& alloc) noexcept
        : arraySize(std::move(other.arraySize))
        , allocated(std::move(other.allocated))
        , allocator(alloc)
    {
        _data = allocateArray(other.allocated);
        std::uninitialized_move(other.begin(), other.end(), begin());
        other.deallocateArray(other._data, other.allocated);
        other._data = nullptr;
        other.allocated = 0;
        other.arraySize = 0;
    }
    Array &operator=(const Array &other)
    {
        if (this != &other)
        {
            if (other.arraySize > allocated)
            {
                clear();
            }
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value )
            {
                if (!std::allocator_traits<allocator_type>::is_always_equal::value
                    && allocator != other.allocator)
                {
                    clear();
                }
                allocator = other.allocator;
            }
            if(_data == nullptr)
            {
                _data = allocateArray(other.allocated);
                allocated = other.allocated;
            }
            arraySize = other.arraySize;
            std::uninitialized_copy(other.begin(), other.end(), begin());
        }
        return *this;
    }
    Array &operator=(Array &&other) noexcept(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value
        || std::allocator_traits<Allocator>::is_always_equal::value)
    {
        if (this != &other)
        {
            if (_data != nullptr)
            {
                clear();
            }
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
            {
                allocator = std::move(other.allocator);
            }
            allocated = std::move(other.allocated);
            arraySize = std::move(other.arraySize);
            _data = other._data;
            other._data = nullptr;
        }
        return *this;
    }
    constexpr ~Array()
    {
        clear();
    }
    
    [[nodiscard]]
    constexpr iterator find(const value_type &item) noexcept
    {
        for (size_type i = 0; i < arraySize; ++i)
        {
            if (_data[i] == item)
            {
                return iterator(&_data[i]);
            }
        }
        return end();
    }
    [[nodiscard]]
    constexpr iterator find(value_type&& item) noexcept
    {
        for (size_type i = 0; i < arraySize; ++i)
        {
            if (_data[i] == item)
            {
                return iterator(&_data[i]);
            }
        }
        return end();	
    }
    [[nodiscard]]
    constexpr const_iterator find(const value_type &item) const noexcept
    {
        for (size_type i = 0; i < arraySize; ++i)
        {
            if (_data[i] == item)
            {
                return const_iterator(&_data[i]);
            }
        }
        return end();
    }
    [[nodiscard]]
    constexpr const_iterator find(value_type&& item) const noexcept
    {
        for (size_type i = 0; i < arraySize; ++i)
        {
            if (_data[i] == item)
            {
                return const_iterator(&_data[i]);
            }
        }
        return end();	
    }
    template<class Pred>
    requires std::predicate<Pred, value_type>
    constexpr const_iterator find(Pred pred) const noexcept
    {
        for (size_type i = 0; i < arraySize; ++i)
        {
            if(pred(_data[i]))
            {
                return const_iterator(&_data[i]);
            }
        }
        return end();
    }

    template<class Pred>
    requires std::predicate<Pred, value_type>
    constexpr iterator find(Pred pred) noexcept
    {
        for (size_type i = 0; i < arraySize; ++i)
        {
            if (pred(_data[i]))
            {
                return iterator(&_data[i]);
            }
        }
        return end();
    }
    constexpr allocator_type get_allocator() const noexcept
    {
        return allocator;
    }
    constexpr iterator begin() noexcept
    {
        return iterator(_data);
    }
    constexpr iterator end() noexcept
    {
        return iterator(_data + arraySize);
    }
    constexpr const_iterator begin() const noexcept
    {
        return const_iterator(_data);
    }
    constexpr const_iterator end() const noexcept
    {
        return const_iterator(_data + arraySize);
    }
    constexpr const_iterator cbegin() const noexcept
    {
        return const_iterator(_data);
    }
    constexpr const_iterator cend() const noexcept
    {
        return const_iterator(_data + arraySize);
    }
    constexpr reference add(const value_type &item = value_type())
    {
        return addInternal(item);
    }
    constexpr reference add(value_type&& item)
    {
        return addInternal(std::forward<T>(item));
    }
    constexpr void addAll(Array other)
    {
        for(auto value : other)
        {
            addInternal(value);
        }
    }
    constexpr reference addUnique(const value_type &item = value_type())
    {
        iterator it;
        if((it = std::move(find(item))) != end())
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
            T *tempArray = allocateArray(allocated);
            assert(tempArray != nullptr);
            
            std::uninitialized_move(begin(), end(), Iterator(tempArray));
            deallocateArray(_data, arraySize);
            _data = tempArray;
        }
        std::allocator_traits<allocator_type>::construct(allocator, &_data[arraySize++], arguments...);
        return _data[arraySize - 1];
    }
    template<class Pred>
    requires std::predicate<Pred, value_type>
    constexpr void remove_if(Pred pred, bool keepOrder = true)
    {
        remove(find(pred), keepOrder);
    }
    constexpr void remove(const value_type& element, bool keepOrder = true)
    {
        remove(find(element), keepOrder);
    }
    constexpr void remove(value_type&& element, bool keepOrder = true)
    {
        remove(find(element), keepOrder);
    }
    constexpr void remove(iterator it, bool keepOrder = true)
    {
        removeAt(it - begin(), keepOrder);
    }
    constexpr void remove(const_iterator it, bool keepOrder = true)
    {
        removeAt(it - cbegin(), keepOrder);
    }
    constexpr void removeAt(size_type index, bool keepOrder = true)
    {
        if (keepOrder)
        {
            for(size_type i = index; i < arraySize-1; ++i)
            {
                _data[i] = std::move(_data[i+1]);
            }
        }
        else
        {
            _data[index] = std::move(_data[arraySize - 1]);
        }
        std::allocator_traits<allocator_type>::destroy(allocator, &_data[--arraySize]);
    }
    constexpr void resize(size_type newSize)
    {
        resizeInternal(newSize, T());
    }
    constexpr void resize(size_type newSize, const value_type& value)
    {
        resizeInternal(newSize, value);
    }
    constexpr void clear() noexcept
    {
        if(_data == nullptr)
        {
            return;
        }
        for(size_type i = 0; i < arraySize; ++i)
        {
            std::allocator_traits<allocator_type>::destroy(allocator, &_data[i]);
        }
        deallocateArray(_data, allocated);
        _data = nullptr;
        arraySize = 0;
        allocated = 0;
    }
    constexpr size_type indexOf(iterator iterator)
    {
        return iterator - begin();
    }
    constexpr size_type indexOf(const_iterator iterator) const
    {
        return iterator.p - begin().p;
    }
    constexpr size_type indexOf(T& t)
    {
        return indexOf(find(t));
    }
    constexpr size_type indexOf(const T& t) const
    {
        return indexOf(find(t));
    }
    constexpr size_type size() const noexcept
    {
        return arraySize;
    }
    [[nodiscard]]
    constexpr bool empty() const noexcept
    {
        return arraySize == 0;
    }
    constexpr void reserve(size_type new_cap)
    {
        if(new_cap > allocated)
        {
            T* temp = allocateArray(new_cap);
            std::uninitialized_move_n(begin(), arraySize, temp);
            _data = temp;
        }
        allocated = new_cap;
    }
    constexpr size_type capacity() const noexcept
    {
        return allocated;
    }
    constexpr pointer data() const noexcept
    {
        return _data;
    }
    constexpr reference front() const
    {
        assert(arraySize > 0);
        return _data[0];
    }
    constexpr reference back() const
    {
        assert(arraySize > 0);
        return _data[arraySize - 1];
    }
    constexpr void pop()
    {
        std::allocator_traits<allocator_type>::destroy(allocator, &_data[--arraySize]);
    }
    constexpr reference operator[](size_type index)
    {
        assert(index < arraySize);
        return _data[index];
    }
    constexpr const reference operator[](size_type index) const
    {
        assert(index < arraySize);
        return _data[index];
    }
private:
    size_type calculateGrowth(size_type newSize) const
    {
        const size_type oldCapacity = capacity();

        if (oldCapacity > SIZE_MAX - oldCapacity)
        {
            return newSize; // geometric growth would overflow
        }

        const size_type geometric = oldCapacity + oldCapacity;

        if (geometric < newSize)
        {
            return newSize; // geometric growth would be insufficient
        }

        return geometric; // geometric growth is sufficient
    }
    [[nodiscard]]
    T* allocateArray(size_type size)
    {
        T* result = allocator.allocate(size);
        assert(result != nullptr);
        return result;
    }
    void deallocateArray(T* ptr, size_type size)
    {
        allocator.deallocate(ptr, size);
    }
    template<typename Type>
    T& addInternal(Type&& t) noexcept
    {
        if (arraySize == allocated)
        {
            size_type newSize = arraySize + 1;
            allocated = calculateGrowth(newSize);
            T *tempArray = allocateArray(allocated);
            for (size_type i = 0; i < arraySize; ++i)
            {
                std::allocator_traits<allocator_type>::construct(allocator, &tempArray[i], std::forward<Type>(_data[i]));
            }
            deallocateArray(_data, arraySize);
            _data = tempArray;
        }
        std::allocator_traits<allocator_type>::construct(allocator, &_data[arraySize++], std::forward<Type>(t));
        return _data[arraySize - 1];
    }
    template<typename Type>
    void resizeInternal(size_type newSize, const Type& value) noexcept
    {
        if (newSize <= allocated)
        {
            // The array is already big enough
            if(newSize < arraySize)
            {
                // But since we are sizing down we destruct some of them
                for(size_type i = newSize; i < arraySize; ++i)
                {
                    std::allocator_traits<allocator_type>::destroy(allocator, &_data[i]);
                }
            }
            else
            {
                // Or construct the new elements by default
                for(size_type i = arraySize; i < newSize; ++i)
                {
                    std::allocator_traits<allocator_type>::construct(allocator, &_data[i], value);
                }
            }
            arraySize = newSize;
        }
        else
        {
            // The array is not big enough, so we make a new one
            T *newData = allocateArray(newSize);

            if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>)
            {
                std::memcpy(newData, _data, sizeof(T) * arraySize);
                std::memset(&newData[arraySize], 0, sizeof(T) * (newSize - arraySize));
            }
            else
            {
                // And move the current elements into that one
                for(size_type i = 0; i < arraySize; ++i)
                {
                    newData[i] = std::forward<Type>(_data[i]);
                }
                // As well as default initialize the others
                for (size_type i = arraySize; i < newSize; ++i)
                {
                    std::allocator_traits<allocator_type>::construct(allocator, &newData[i], value);
                }
            }
            deallocateArray(_data, allocated);
            arraySize = newSize;
            allocated = newSize;
            _data = newData;
        }
    }
    size_type arraySize = 0;
    size_type allocated = 0;
    T *_data = nullptr;
    allocator_type allocator;
};


template<class Type, class Alloc>
constexpr bool operator==(const Array<Type, Alloc> &lhs, const Array<Type, Alloc>& rhs)
{
    if(lhs.size() != rhs.size())
    {
        return false;
    }
    for(auto it1 = lhs.begin(), it2 = rhs.begin(); it1 != lhs.end() && it2 != rhs.end(); ++it1, ++it2)
    {
        if(*it1 != *it2)
        {
            return false;
        }
    }
    return true;
}

template<class Type, class Alloc>
constexpr auto operator<=>(const Array<Type, Alloc>& lhs, const Array<Type, Alloc>& rhs)
{
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, size_t N>
struct StaticArray
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

        IteratorBase(X *x = nullptr)
            : p(x)
        {
        }
        reference operator*() const
        {
            return *p;
        }
        pointer operator->() const
        {
            return p;
        }
        inline bool operator!=(const IteratorBase &other)
        {
            return p != other.p;
        }
        inline bool operator==(const IteratorBase &other)
        {
            return p == other.p;
        }
        IteratorBase &operator++()
        {
            p++;
            return *this;
        }
        IteratorBase operator++(int)
        {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }
        IteratorBase &operator--()
        {
            p--;
            return *this;
        }
        IteratorBase operator--(int)
        {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }
        

    private:
        X *p;
    };
    using value_type = T;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using iterator = IteratorBase<T>;
    using const_iterator = IteratorBase<const T>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    StaticArray()
    {
        beginIt = iterator(_data);
        endIt = iterator(_data + N);
    }
    StaticArray(T value)
    {
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] = value;
        }
        beginIt = iterator(_data);
        endIt = iterator(_data + N);
    }
    StaticArray(std::initializer_list<T> init)
    {
        auto beg = init.begin();
        for (size_t i = 0; i < N; ++i)
        {
            _data[i] = *beg;
            if (init.size() == N)
            {
                beg++;
            }
        }
    }
    ~StaticArray()
    {
    }
    
    inline size_type size() const
    {
        return N;
    }
    inline pointer data()
    {
        return _data;
    }
    inline const_pointer data() const
    {
        return _data;
    }
    template<typename I>
    constexpr reference operator[](I index) noexcept
    {
        return operator[](static_cast<size_t>(index));
    }
    template<typename I>
    constexpr const_reference operator[](I index) const noexcept
    {
        return operator[](static_cast<size_t>(index));
    }
    constexpr reference operator[](size_type index) noexcept
    {
        assert(index < N);
        return _data[index];
    }
    constexpr const_reference operator[](size_type index) const noexcept
    {
        assert(index < N);
        return _data[index];
    }
    iterator begin()
    {
        return beginIt;
    }
    iterator end()
    {
        return endIt;
    }
    const_iterator begin() const
    {
        return beginIt;
    }
    const_iterator end() const
    {
        return beginIt;
    }
private:
    T _data[N];
    iterator beginIt;
    iterator endIt;
};
} // namespace Seele