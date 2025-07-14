#pragma once
#include "Concepts.h"
#include "EngineTypes.h"
#include "MemoryResource.h"
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory_resource>

namespace Seele {
template <typename T> struct Array {
  public:
    template <typename X> class IteratorBase {
      public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = X;
        using difference_type = std::ptrdiff_t;
        using reference = X&;
        using pointer = X*;

        constexpr IteratorBase(X* x = nullptr) : p(x) {}
        constexpr reference operator*() const { return *p; }
        constexpr pointer operator->() const { return p; }
        constexpr bool operator==(const IteratorBase& other) const { return p == other.p; }
        constexpr bool operator!=(const IteratorBase& other) const { return p != other.p; }
        constexpr std::strong_ordering operator<=>(const IteratorBase& other) const { return p <=> other.p; }
        constexpr IteratorBase operator+(size_t other) const {
            IteratorBase tmp(*this);
            tmp.p += other;
            return tmp;
        }
        constexpr int operator-(const IteratorBase& other) const { return (int)(p - other.p); }
        constexpr IteratorBase& operator-=(difference_type other) {
            p -= other;
            return *this;
        }
        constexpr IteratorBase& operator+=(difference_type other) {
            p += other;
            return *this;
        }
        constexpr IteratorBase operator-(difference_type diff) const { return IteratorBase(p - diff); }
        constexpr IteratorBase& operator++() {
            p++;
            return *this;
        }
        constexpr IteratorBase& operator--() {
            p--;
            return *this;
        }
        constexpr IteratorBase operator++(int) {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }
        constexpr IteratorBase operator--(int) {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }

      private:
        X* p;
    };

    using value_type = T;
    using allocator_type = std::pmr::polymorphic_allocator<T>;
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

    constexpr Array(const allocator_type& alloc = allocator_type()) noexcept
        : arraySize(0), allocated(0), _data(nullptr), allocator(alloc) {}
    constexpr Array(size_type size, const value_type& value, const allocator_type& alloc = allocator_type())
        : arraySize(size), allocated(size), allocator(alloc) {
        _data = allocateArray(size);
        assert(_data != nullptr);
        for (size_type i = 0; i < size; ++i) {
            std::allocator_traits<allocator_type>::construct(allocator, &_data[i], value);
        }
    }
    constexpr explicit Array(size_type size, const allocator_type& alloc = allocator_type())
        : arraySize(size), allocated(size), allocator(alloc) {
        _data = allocateArray(size);
        assert(_data != nullptr);
        if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
            std::memset(_data, 0, size * sizeof(T));
        } else {
            for (size_type i = 0; i < size; ++i) {
                std::allocator_traits<allocator_type>::construct(allocator, &_data[i]);
            }
        }
    }
    constexpr Array(std::initializer_list<T> init, const allocator_type& alloc = allocator_type())
        : arraySize(init.size()), allocated(init.size()), allocator(alloc) {
        _data = allocateArray(init.size());
        assert(_data != nullptr);
        std::uninitialized_copy(init.begin(), init.end(), begin());
    }
    template <container_compatible_range<T> Range>
    constexpr Array(std::from_range_t, Range&& rg, const allocator_type& alloc = allocator_type())
        : arraySize(0), allocated(0), allocator(alloc) {
        if constexpr (std::ranges::sized_range<Range> || std::ranges::forward_range<Range>) {
            arraySize = std::ranges::size(rg);
            allocated = arraySize;
            _data = allocateArray(allocated);
            std::uninitialized_copy(rg.begin(), rg.end(), begin());
        } else {
            for (auto it = std::ranges::begin(rg); it != std::ranges::end(rg); it++) {
                add(*it);
            }
        }
    }
    constexpr Array(const Array& other)
        : arraySize(other.arraySize), allocated(other.allocated),
          allocator(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.allocator)) {
        _data = allocateArray(other.allocated);
        assert(_data != nullptr);
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }
    constexpr Array(const Array& other, const allocator_type& alloc)
        : arraySize(other.arraySize), allocated(other.allocated), allocator(alloc) {
        _data = allocateArray(other.allocated);
        assert(_data != nullptr);
        std::uninitialized_copy(other.begin(), other.end(), begin());
    }
    constexpr Array(Array&& other) noexcept
        : arraySize(std::move(other.arraySize)), allocated(std::move(other.allocated)), allocator(std::move(other.allocator)) {
        _data = other._data;
        other._data = nullptr;
        other.allocated = 0;
        other.arraySize = 0;
    }
    constexpr Array(Array&& other, const allocator_type& alloc) noexcept
        : arraySize(std::move(other.arraySize)), allocated(std::move(other.allocated)), allocator(alloc) {
        _data = allocateArray(other.allocated);
        std::uninitialized_move(other.begin(), other.end(), begin());
        other.deallocateArray(other._data, other.allocated);
        other._data = nullptr;
        other.allocated = 0;
        other.arraySize = 0;
    }
    Array& operator=(const Array& other) {
        if (this != &other) {
            if (other.arraySize > allocated) {
                clear();
            }
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
                if (!std::allocator_traits<allocator_type>::is_always_equal::value && allocator != other.allocator) {
                    clear();
                }
                allocator = other.allocator;
            }
            if (_data == nullptr) {
                _data = allocateArray(other.allocated);
                allocated = other.allocated;
            }
            arraySize = other.arraySize;
            std::uninitialized_copy(other.begin(), other.end(), begin());
        }
        return *this;
    }
    Array& operator=(Array&& other) noexcept(std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value ||
                                             std::allocator_traits<allocator_type>::is_always_equal::value) {
        if (this != &other) {
            if (_data != nullptr) {
                clear();
            }
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
                allocator = std::move(other.allocator);
            }
            allocated = std::move(other.allocated);
            arraySize = std::move(other.arraySize);
            _data = other._data;
            other._data = nullptr;
        }
        return *this;
    }
    constexpr ~Array() { clear(); }

    [[nodiscard]] constexpr iterator find(const value_type& item) noexcept {
        for (size_type i = 0; i < arraySize; ++i) {
            if (_data[i] == item) {
                return iterator(&_data[i]);
            }
        }
        return end();
    }
    [[nodiscard]] constexpr iterator find(value_type&& item) noexcept {
        for (size_type i = 0; i < arraySize; ++i) {
            if (_data[i] == item) {
                return iterator(&_data[i]);
            }
        }
        return end();
    }
    [[nodiscard]] constexpr const_iterator find(const value_type& item) const noexcept {
        for (size_type i = 0; i < arraySize; ++i) {
            if (_data[i] == item) {
                return const_iterator(&_data[i]);
            }
        }
        return end();
    }
    [[nodiscard]] constexpr const_iterator find(value_type&& item) const noexcept {
        for (size_type i = 0; i < arraySize; ++i) {
            if (_data[i] == item) {
                return const_iterator(&_data[i]);
            }
        }
        return end();
    }
    template <class Pred>
        requires std::predicate<Pred, value_type>
    constexpr const_iterator find(Pred pred) const noexcept {
        for (size_type i = 0; i < arraySize; ++i) {
            if (pred(_data[i])) {
                return const_iterator(&_data[i]);
            }
        }
        return end();
    }

    template <class Pred>
        requires std::predicate<Pred, value_type>
    constexpr iterator find(Pred&& pred) noexcept {
        for (size_type i = 0; i < arraySize; ++i) {
            if (pred(_data[i])) {
                return iterator(&_data[i]);
            }
        }
        return end();
    }
    constexpr allocator_type get_allocator() const noexcept { return allocator; }
    constexpr iterator begin() noexcept { return iterator(_data); }
    constexpr iterator end() noexcept { return iterator(_data + arraySize); }
    constexpr const_iterator begin() const noexcept { return const_iterator(_data); }
    constexpr const_iterator end() const noexcept { return const_iterator(_data + arraySize); }
    constexpr const_iterator cbegin() const noexcept { return const_iterator(_data); }
    constexpr const_iterator cend() const noexcept { return const_iterator(_data + arraySize); }
    constexpr reference add(const value_type& item = value_type()) { return addInternal(item); }
    constexpr reference add(value_type&& item) { return addInternal(std::forward<T>(item)); }
    constexpr void addAll(const Array& other) {
        for (const auto& value : other) {
            addInternal(value);
        }
    }
    constexpr void addAll(Array&& other) {
        for (auto&& value : other) {
            addInternal(value);
        }
    }
    constexpr reference addUnique(const value_type& item = value_type()) {
        iterator it;
        if ((it = std::move(find(item))) != end()) {
            return *it;
        }
        return addInternal(item);
    }
    template <typename... args> constexpr reference emplace(args... arguments) {
        if (arraySize == allocated) {
            size_type newSize = arraySize + 1;
            allocated = calculateGrowth(newSize);
            T* tempArray = allocateArray(allocated);
            assert(tempArray != nullptr);

            std::uninitialized_move(begin(), end(), Iterator(tempArray));
            deallocateArray(_data, arraySize);
            _data = tempArray;
        }
        std::allocator_traits<allocator_type>::construct(allocator, &_data[arraySize++], arguments...);
        return _data[arraySize - 1];
    }
    template <class Pred>
        requires std::predicate<Pred, value_type>
    constexpr void remove_if(Pred&& pred, bool keepOrder = true) {
        Iterator it;
        while ((it = find(pred)) != end()) {
            erase(it, keepOrder);
        }
    }
    constexpr void remove(const value_type& element, bool keepOrder = true) { erase(find(element), keepOrder); }
    constexpr void remove(value_type&& element, bool keepOrder = true) { erase(find(element), keepOrder); }
    constexpr void erase(iterator it, bool keepOrder = true) { removeAt(it - begin(), keepOrder); }
    constexpr void erase(const_iterator it, bool keepOrder = true) { removeAt(it - cbegin(), keepOrder); }
    constexpr bool contains(const T& value) const { return find(value) != end(); }
    constexpr void removeAt(size_type index, bool keepOrder = true) {
        if (keepOrder) {
            for (size_type i = index; i < arraySize - 1; ++i) {
                _data[i] = std::move(_data[i + 1]);
            }
        } else {
            _data[index] = std::move(_data[arraySize - 1]);
        }
        std::allocator_traits<allocator_type>::destroy(allocator, &_data[--arraySize]);
    }
    constexpr void resize(size_type newSize) { resizeInternal(newSize, T()); }
    constexpr void resize(size_type newSize, const value_type& value) { resizeInternal(newSize, value); }
    constexpr void clear(bool preserveAllocation = false) noexcept {
        if (_data == nullptr) {
            return;
        }
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_type i = 0; i < arraySize; ++i) {
                std::allocator_traits<allocator_type>::destroy(allocator, &_data[i]);
            }
        }
        if (!preserveAllocation) {
            deallocateArray(_data, allocated);
            _data = nullptr;
            allocated = 0;
        }
        arraySize = 0;
    }
    [[nodiscard]] constexpr size_type indexOf(iterator iterator) { return iterator - begin(); }
    [[nodiscard]] constexpr size_type indexOf(const_iterator iterator) const { return iterator.p - begin().p; }
    [[nodiscard]] constexpr size_type indexOf(T& t) { return indexOf(find(t)); }
    [[nodiscard]] constexpr size_type indexOf(const T& t) const { return indexOf(find(t)); }
    [[nodiscard]] constexpr size_type size() const noexcept { return arraySize; }
    [[nodiscard]] constexpr bool empty() const noexcept { return arraySize == 0; }
    constexpr void reserve(size_type new_cap) {
        if (new_cap > allocated) {
            T* temp = allocateArray(new_cap);
            if constexpr (std::is_trivially_copyable_v<T>) {
                std::memcpy(temp, _data, sizeof(T) * arraySize);
            } else {
                std::uninitialized_move_n(begin(), arraySize, temp);
            }
            deallocateArray(_data, allocated);
            _data = temp;
        }
        allocated = new_cap;
    }
    constexpr size_type capacity() const noexcept { return allocated; }
    constexpr pointer data() const noexcept { return _data; }
    constexpr reference front() const {
        assert(arraySize > 0);
        return _data[0];
    }
    constexpr reference back() const {
        assert(arraySize > 0);
        return _data[arraySize - 1];
    }
    constexpr void pop() { std::allocator_traits<allocator_type>::destroy(allocator, &_data[--arraySize]); }
    constexpr reference operator[](size_type index) {
        assert(index < arraySize);
        return _data[index];
    }
    constexpr const_reference operator[](size_type index) const {
        assert(index < arraySize);
        return _data[index];
    }

  private:
    size_type calculateGrowth(size_type newSize) const {
        const size_type oldCapacity = capacity();

        if (oldCapacity > SIZE_MAX - oldCapacity) {
            return newSize; // geometric growth would overflow
        }

        const size_type geometric = oldCapacity + oldCapacity;

        if (geometric < newSize) {
            return newSize; // geometric growth would be insufficient
        }

        return geometric; // geometric growth is sufficient
    }
    [[nodiscard]] T* allocateArray(size_type size) {
        T* result = allocator.allocate(size);
        assert(result != nullptr);
        return result;
    }
    void deallocateArray(T* ptr, size_type size) {
        if (ptr == nullptr)
            return;
        allocator.deallocate(ptr, size);
    }
    template <typename Type> T& addInternal(Type&& t) noexcept {
        if (arraySize == allocated) {
            size_type newSize = arraySize + 1;
            allocated = calculateGrowth(newSize);
            T* tempArray = allocateArray(allocated);
            for (size_type i = 0; i < arraySize; ++i) {
                std::allocator_traits<allocator_type>::construct(allocator, &tempArray[i], std::forward<Type>(_data[i]));
            }
            if (_data != nullptr)
            {
                deallocateArray(_data, arraySize);
            }
            _data = tempArray;
        }
        std::allocator_traits<allocator_type>::construct(allocator, &_data[arraySize], std::forward<Type>(t));
        return _data[arraySize++];
    }
    template <typename Type> void resizeInternal(size_type newSize, const Type& value) noexcept {
        if (newSize <= allocated) {
            // The array is already big enough
            if (newSize < arraySize) {
                // But since we are sizing down we destruct some of them

                if constexpr (!std::is_trivially_destructible_v<T>) {
                    for (size_type i = newSize; i < arraySize; ++i) {
                        std::allocator_traits<allocator_type>::destroy(allocator, &_data[i]);
                    }
                }
            } else {
                // Or construct the new elements by default
                for (size_type i = arraySize; i < newSize; ++i) {
                    std::allocator_traits<allocator_type>::construct(allocator, &_data[i], value);
                }
            }
            arraySize = newSize;
        } else {
            size_t oldSize = allocated;
            allocated = calculateGrowth(newSize);
            // The array is not big enough, so we make a new one
            T* newData = allocateArray(allocated);

            if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
                std::memcpy(newData, _data, sizeof(T) * arraySize);
                std::memset(&newData[arraySize], 0, sizeof(T) * (allocated - arraySize));
            } else {
                // And move the current elements into that one
                std::uninitialized_move(begin(), end(), Iterator(newData));
                // As well as default initialize the others
                for (size_type i = arraySize; i < allocated; ++i) {
                    std::allocator_traits<allocator_type>::construct(allocator, &newData[i], value);
                }
            }
            deallocateArray(_data, oldSize);
            arraySize = newSize;
            _data = newData;
        }
    }
    size_type arraySize = 0;
    size_type allocated = 0;
    T* _data = nullptr;
    allocator_type allocator;
};

template <class Type> constexpr bool operator==(const Array<Type>& lhs, const Array<Type>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (auto it1 = lhs.begin(), it2 = rhs.begin(); it1 != lhs.end() && it2 != rhs.end(); ++it1, ++it2) {
        if (*it1 != *it2) {
            return false;
        }
    }
    return true;
}

template <class Type> constexpr auto operator<=>(const Array<Type>& lhs, const Array<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, size_t N> struct StaticArray {
  public:
    template <typename X> class IteratorBase {
      public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = X;
        using difference_type = std::ptrdiff_t;
        using reference = X&;
        using pointer = X*;

        constexpr IteratorBase(X* x = nullptr) : p(x) {}
        constexpr reference operator*() const { return *p; }
        constexpr pointer operator->() const { return p; }
        constexpr bool operator!=(const IteratorBase& other) { return p != other.p; }
        constexpr bool operator==(const IteratorBase& other) { return p == other.p; }
        constexpr IteratorBase& operator++() {
            p++;
            return *this;
        }
        constexpr IteratorBase operator++(int) {
            IteratorBase tmp(*this);
            ++*this;
            return tmp;
        }
        constexpr IteratorBase& operator--() {
            p--;
            return *this;
        }
        constexpr IteratorBase operator--(int) {
            IteratorBase tmp(*this);
            --*this;
            return tmp;
        }

      private:
        X* p;
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

    constexpr StaticArray() {}
    
    constexpr StaticArray(T value) {
        for (size_t i = 0; i < N; ++i) {
            _data[i] = value;
        }
    }
    
    constexpr StaticArray(std::initializer_list<T> init) {
        auto beg = init.begin();
        for (size_t i = 0; i < N; ++i) {
            _data[i] = *beg;
            if (init.size() == N) {
                beg++;
            }
        }
    }
    constexpr StaticArray(const StaticArray& other) = default;
    constexpr StaticArray(StaticArray&& other) = default;
    constexpr ~StaticArray() {}
    constexpr StaticArray& operator=(const StaticArray& other) = default;
    constexpr StaticArray& operator=(StaticArray&& other) = default;
    constexpr size_type size() const { return N; }
    constexpr pointer data() { return _data; }
    constexpr const_pointer data() const { return _data; }
    template <typename I> constexpr reference operator[](I index) noexcept { return operator[](static_cast<size_t>(index)); }
    template <typename I> constexpr const_reference operator[](I index) const noexcept { return operator[](static_cast<size_t>(index)); }
    constexpr reference operator[](size_type index) noexcept {
        assert(index < N);
        return _data[index];
    }
    constexpr const_reference operator[](size_type index) const noexcept {
        assert(index < N);
        return _data[index];
    }
    constexpr iterator begin() { return iterator(_data); }
    constexpr iterator end() { return iterator(_data + N); }
    constexpr const_iterator begin() const { return iterator(_data); }
    constexpr const_iterator end() const { return iterator(_data + N); }

  private:
    T _data[N] = {T()};
};
} // namespace Seele
