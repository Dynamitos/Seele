#pragma once
#include "EngineTypes.h"
#include <initializer_list>
#include <iterator>
#include <assert.h>
#include <memory_resource>
#include <boost/serialization/serialization.hpp>

#ifndef DEFAULT_ALLOC_SIZE
#define DEFAULT_ALLOC_SIZE 16
#endif

namespace Seele
{
	template <typename T>
	struct Array
	{
	public:
		Array()
			: allocated(DEFAULT_ALLOC_SIZE)
			, arraySize(0)
		{
			_data = new T[DEFAULT_ALLOC_SIZE];
			assert(_data != nullptr);
			//std::memset(_data, 0, sizeof(T) * DEFAULT_ALLOC_SIZE);
			refreshIterators();
		}
		Array(size_t size, T value = T())
			: allocated(size), arraySize(size)
		{
			_data = new T[size];
			assert(_data != nullptr);
			for (size_t i = 0; i < size; ++i)
			{
				assert(i < size);
				_data[i] = value;
			}
			refreshIterators();
		}
		Array(std::initializer_list<T> init)
			: allocated(init.size()), arraySize(init.size())
		{
			_data = new T[init.size()];
			auto it = init.begin();
			for (size_t i = 0; it != init.end(); i++, it++)
			{
				assert(i < init.size());
				_data[i] = *it;
			}
			refreshIterators();
		}
		Array(const Array &other)
			: allocated(other.allocated), arraySize(other.arraySize)
		{
			_data = new T[other.allocated];
			assert(_data != nullptr);
			refreshIterators();
			std::copy(other.begin(), other.end(), beginIt);
		}
		Array(Array &&other) noexcept
			: allocated(std::move(other.allocated)), arraySize(std::move(other.arraySize))
		{
			_data = other._data;
			other._data = nullptr;
			other.allocated = 0;
			other.arraySize = 0;
			refreshIterators();
		}
		Array &operator=(const Array &other) noexcept
		{
			if (this != &other)
			{
				if (_data != nullptr)
				{
					delete[] _data;
				}
				allocated = other.allocated;
				arraySize = other.arraySize;
				_data = new T[other.allocated];
				refreshIterators();
				std::copy(other.begin(), other.end(), beginIt);
			}
			return *this;
		}
		Array &operator=(Array &&other) noexcept
		{
			if (this != &other)
			{
				if (_data != nullptr)
				{
					delete[] _data;
				}
				allocated = std::move(other.allocated);
				arraySize = std::move(other.arraySize);
				_data = other._data;
				other._data = nullptr;
				refreshIterators();
			}
			return *this;
		}
		~Array()
		{
			if (_data)
			{
				delete[] _data;
				_data = nullptr;
			}
		}
		template <typename X>
		class IteratorBase
		{
		public:
			typedef std::forward_iterator_tag iterator_category;
			typedef X value_type;
			typedef std::ptrdiff_t difference_type;
			typedef X &reference;
			typedef X *pointer;

			IteratorBase(X *x = nullptr)
				: p(x)
			{
			}
			IteratorBase(const IteratorBase &i)
				: p(i.p)
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
			inline int operator-(const IteratorBase &other)
			{
				return (int)(p - other.p);
			}
			IteratorBase &operator++()
			{
				p++;
				return *this;
			}
			IteratorBase &operator--()
			{
				p--;
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
			X *p;
		};
		typedef IteratorBase<T> Iterator;
		typedef IteratorBase<const T> ConstIterator;

		bool operator==(const Array &other)
		{
			return _data == other._data;
		}

		bool operator!=(const Array &other)
		{
			return !(*this == other);
		}

		Iterator find(const T &item)
		{
			for (uint32 i = 0; i < arraySize; ++i)
			{
				if (_data[i] == item)
				{
					return Iterator(&_data[i]);
				}
			}
			return endIt;
		}
		Iterator find(T&& item)
		{
			for (uint32 i = 0; i < arraySize; ++i)
			{
				if (_data[i] == item)
				{
					return Iterator(&_data[i]);
				}
			}
			return endIt;	
		}
		Iterator begin() const
		{
			return beginIt;
		}
		Iterator end() const
		{
			return endIt;
		}
		ConstIterator cbegin() const
		{
			return beginIt;
		}
		ConstIterator cend() const
		{
			return endIt;
		}

		T &add(const T &item = T())
		{
			if (arraySize == allocated)
			{
				size_t newSize = arraySize + 1;
				allocated = calculateGrowth(newSize);
				T *tempArray = new T[allocated];
				assert(tempArray != nullptr);
				for (size_t i = 0; i < arraySize; ++i)
				{
					tempArray[i] = _data[i];
				}
				delete[] _data;
				_data = tempArray;
			}
			_data[arraySize++] = item;
			refreshIterators();
			return _data[arraySize - 1];
		}
		T &add(T&& item)
		{
			if (arraySize == allocated)
			{
				size_t newSize = arraySize + 1;
				allocated = calculateGrowth(newSize);
				T *tempArray = new T[allocated];
				assert(tempArray != nullptr);
				for (size_t i = 0; i < arraySize; ++i)
				{
					tempArray[i] = std::move(_data[i]);
				}
				delete[] _data;
				_data = tempArray;
			}
			_data[arraySize++] = std::move(item);
			refreshIterators();
			return _data[arraySize - 1];
		}
		T &addUnique(const T &item = T())
		{
			Iterator it;
			if((it = find(item)) != endIt)
			{
				return *it;
			}
			return add(item);
		}
		template<typename... args>
		T &emplace(args... arguments)
		{
			if (arraySize == allocated)
			{
				size_t newSize = arraySize + 1;
				allocated = calculateGrowth(newSize);
				T *tempArray = new T[allocated];
				assert(tempArray != nullptr);
				for (size_t i = 0; i < arraySize; ++i)
				{
					tempArray[i] = _data[i];
				}
				delete[] _data;
				_data = tempArray;
			}
			_data[arraySize++] = T(arguments...);
			refreshIterators();
			return _data[arraySize - 1];
		}
		void remove(Iterator it, bool keepOrder = true)
		{
			remove(it - beginIt, keepOrder);
		}
		void remove(int index, bool keepOrder = true)
		{
			if (keepOrder)
			{
				std::memcpy(&_data[index], &_data[index + 1], sizeof(T) * (arraySize - index));
			}
			else
			{
				_data[index] = std::move(_data[arraySize - 1]);
			}
			arraySize--;
		}
		void clear()
		{
			delete[] _data;
			_data = nullptr;
			arraySize = 0;
			allocated = 0;
			refreshIterators();
		}
		void resize(size_t newSize)
		{
			if (newSize < allocated)
			{
				arraySize = newSize;
			}
			else
			{
				T *newData = new T[newSize];
				assert(newData != nullptr);
				allocated = newSize;
				std::memcpy(newData, _data, sizeof(T) * arraySize);
				arraySize = newSize;
				delete _data;
				_data = newData;
			}
			refreshIterators();
		}
		inline size_t indexOf(Iterator iterator)
		{
			return iterator - beginIt;
		}
		inline size_t indexOf(ConstIterator iterator) const
		{
			return iterator.p - beginIt.p;
		}
		inline size_t indexOf(T& t)
		{
			return indexOf(find(t));
		}
		inline size_t indexOf(const T& t) const
		{
			return indexOf(find(t));
		}
		inline size_t size() const
		{
			return arraySize;
		}
		inline size_t empty() const
		{
			return arraySize == 0;
		}
		inline size_t capacity() const
		{
			return allocated;
		}
		inline T *data() const
		{
			return _data;
		}
		T &back() const
		{
			return _data[arraySize - 1];
		}
		void pop()
		{
			arraySize--;
			refreshIterators();
		}
		T &operator[](size_t index)
		{
			assert(index < arraySize);
			return _data[index];
		}
		const T &operator[](size_t index) const
		{
			assert(index < arraySize);
			return _data[index];
		}
	private:
		size_t calculateGrowth(size_t newSize) const
		{
			const size_t oldCapacity = capacity();

			if (oldCapacity > SIZE_MAX - oldCapacity / 2)
			{
				return newSize; // geometric growth would overflow
			}

			const size_t geometric = oldCapacity + oldCapacity / 2;

			if (geometric < newSize)
			{
				return newSize; // geometric growth would be insufficient
			}

			return geometric; // geometric growth is sufficient
		}
		void refreshIterators()
		{
			beginIt = Iterator(_data);
			endIt = Iterator(_data + arraySize);
		}
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & arraySize;
			ar & allocated;
			for(size_t i = 0; i < arraySize; ++i)
				ar & _data[i];
			refreshIterators();
		}
		size_t arraySize;
		size_t allocated;
		Iterator beginIt;
		Iterator endIt;
		T *_data;
	};

	template <typename T, size_t N>
	struct StaticArray
	{
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

			IteratorBase(X *x = nullptr)
				: p(x)
			{
			}
			IteratorBase(const IteratorBase &i)
				: p(i.p)
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
			for (int i = 0; i < N; ++i)
			{
				_data[i] = value;
			}
			beginIt = iterator(_data);
			endIt = iterator(_data + N);
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
		constexpr reference operator[](size_type index) noexcept
		{
			assert(index >= 0 && index < N);
			return _data[index];
		}
		constexpr const_reference operator[](size_type index) const noexcept
		{
			assert(index >= 0 && index < N);
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
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & N;
			ar & _data;
		}
	};
} // namespace Seele