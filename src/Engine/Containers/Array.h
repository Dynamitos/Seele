#pragma once
#include "MinimalEngine.h"
#include <assert.h>

#ifndef DEFAULT_ALLOC_SIZE
#define DEFAULT_ALLOC_SIZE 16
#endif

namespace Seele 
{
	template<typename T>
	struct Array
	{
	public:
		Array()
			: allocated(DEFAULT_ALLOC_SIZE)
			, arraySize(0)
		{
			_data = (T*)malloc(DEFAULT_ALLOC_SIZE * sizeof(T));
			refreshIterators();
		}
		Array(size_t size, T value = T())
			: allocated(size)
			, arraySize(size)
		{
			_data = new T[size];
			for (int i = 0; i < size; ++i)
			{
				_data[i] = value;
			}
			refreshIterators();
		}
		~Array()
		{
			delete _data;
			_data = nullptr;
		}
		template<typename X>
		class IteratorBase {
		public:
			typedef std::forward_iterator_tag iterator_category;
			typedef X value_type;
			typedef std::ptrdiff_t difference_type;
			typedef X& reference;
			typedef X* pointer;

			IteratorBase(X* x = nullptr)
				: p(x)
			{}
			IteratorBase(const IteratorBase& i)
				: p(i.p)
			{}
			reference operator*() const
			{
				return *p;
			}
			pointer operator->() const
			{
				return p;
			}
			inline bool operator!=(const IteratorBase& other)
			{
				return p != other.p;
			}
			inline bool operator==(const IteratorBase& other)
			{
				return p == other.p;
			}
			inline bool operator-(const IteratorBase& other)
			{
				return p - other.p;
			}
			IteratorBase& operator++() {
				p++;
				return *this;
			}
			IteratorBase operator++(int) {
				IteratorBase tmp(*this);
				++*this;
				return tmp;
			}
		private:
			X* p;
		};
		typedef IteratorBase<T> Iterator;
		typedef IteratorBase<const T> ConstIterator;

		Iterator find(const T& item)
		{
			for (int i = 0; i < arraySize; ++i)
			{
				if (_data[i] == item)
				{
					return Iterator(&_data[i]);
				}
			}
			return endIt;
		}
		Iterator begin()
		{
			return beginIt;
		}
		Iterator end()
		{
			return endIt;
		}

		T& add(const T& item)
		{
			if (arraySize == allocated)
			{
				allocated += DEFAULT_ALLOC_SIZE;
				void* tempArray = malloc(sizeof(T) * allocated);
				std::memcpy(tempArray, _data, arraySize * sizeof(T));
				delete _data;
				_data = (T*)tempArray;
			}
			_data[arraySize++] = item;
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
				_data[index] = _data[arraySize - 1];
			}
			arraySize--;
		}
		void clear()
		{
			arraySize = 0;
			allocated = 0;
			refreshIterators();
		}
		void resize(uint32 newSize)
		{
			if (newSize < allocated)
			{
				arraySize = newSize;
			}
			else
			{
				T* newData = new T[newSize];
				allocated = newSize;
				std::memcpy(newData, _data, sizeof(T) * arraySize);
				arraySize = newSize;
				delete _data;
				_data = newData;
			}
			refreshIterators();
		}
		inline uint32 size() const
		{
			return arraySize;
		}
		inline uint32 capacity() const
		{
			return allocated;
		}
		inline T* data() const
		{
			return _data;
		}
		T& back() const
		{
			return _data[arraySize - 1];
		}
		T& operator[](int index) const
		{
			assert(index >= 0 && index < arraySize);
			return _data[index];
		}
	private:
		void refreshIterators()
		{
			beginIt = Iterator(_data);
			endIt = Iterator(_data + arraySize);
		}
		uint32 arraySize;
		uint32 allocated;
		Iterator beginIt;
		Iterator endIt;
		T* _data;
	};

	template<typename T, uint32 N>
	struct StaticArray
	{
	public:
		StaticArray()
		{
			beginIt = Iterator(_data);
			endIt = Iterator(_data + N);
		}
		StaticArray(T value)
		{
			for (int i = 0; i < N; ++i)
			{
				_data[i] = value;
			}
			beginIt = Iterator(_data);
			endIt = Iterator(_data + N);
		}
		~StaticArray()
		{}

		inline uint32 size() const
		{
			return N;
		}
		inline T* data() const
		{
			return _data;
		}
		T& operator[](int index)
		{
			assert(index >= 0 && index < N);
			return _data[index];
		}

		template<typename X>
		class IteratorBase {
		public:
			typedef std::forward_iterator_tag iterator_category;
			typedef X value_type;
			typedef std::ptrdiff_t difference_type;
			typedef X& reference;
			typedef X* pointer;

			IteratorBase(X* x = nullptr)
				: p(x)
			{}
			IteratorBase(const IteratorBase& i)
				: p(i.p)
			{}
			reference operator*() const
			{
				return *p;
			}
			pointer operator->() const
			{
				return p;
			}
			inline bool operator!=(const IteratorBase& other)
			{
				return p != other.p;
			}
			inline bool operator==(const IteratorBase& other)
			{
				return p == other.p;
			}
			IteratorBase& operator++() {
				p++;
				return *this;
			}
			IteratorBase operator++(int) {
				IteratorBase tmp(*this);
				++*this;
				return tmp;
			}
		private:
			X* p;
		};
		typedef IteratorBase<T> Iterator;
		typedef IteratorBase<const T> ConstIterator;

	private:
		T _data[N];
		Iterator beginIt;
		Iterator endIt;
	};
}