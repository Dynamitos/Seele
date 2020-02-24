#pragma once
#include <stdint.h>
#include <array>
#include <memory>
#include <atomic>

namespace Seele
{
	typedef uint64_t uint64;
	typedef uint32_t uint32;
	typedef uint16_t uint16;
	typedef uint8_t uint8;

	typedef int64_t int64;
	typedef int32_t int32;
	typedef int16_t int16;
	typedef int8_t int8;


	template<typename T>
	class RefPtr
	{
	public:
		RefPtr(T* ptr)
		{
			object = new RefObject(ptr);
		}
		RefPtr(const RefPtr& other)
			: object(other.object)
		{
			object->addRef();
		}
		RefPtr& operator=(const RefPtr& other)
		{
			if (this != &other)
			{
				object->removeRef();
				object = other->object;
				object->addRef();
			}
			return *this;
		}
		~RefPtr()
		{
			object->removeRef();
		}
		T* operator->()
		{
			return object->getHandle();
		}
	private:
		class RefObject
		{
		public:
			RefObject(T* ptr)
				: handle(ptr)
				, refCount(1)
			{
			}
			RefObject(const RefObject& rhs)
				: handle(rhs.handle)
				, refCount(rhs.refCount)
			{

			}
			~RefObject()
			{}

			void addRef()
			{
				refCount++;
			}
			void removeRef()
			{
				refCount--;
				if (refCount.load() <= 0)
				{
					delete handle;
					handle = nullptr;
					delete this;
				}
			}
			inline T* getHandle() const
			{
				return handle;
			}
		private:
			T* handle;
			std::atomic_uint64_t refCount;
		};
		RefObject* object;
	};

	template<typename T>
	class UniquePtr
	{
	public:
		UniquePtr(T* ptr)
			: handle(ptr)
		{}
		UniquePtr(const UniquePtr& rhs) = delete;
		UniquePtr(UniquePtr&& rhs) noexcept
			: handle(rhs.handle)
		{
			rhs.handle = nullptr;
		}
		UniquePtr& operator=(const UniquePtr& rhs) = delete;
		UniquePtr& operator=(UniquePtr&& rhs)
		{
			handle = rhs.handle;
			rhs.handle = nullptr;
			return *this;
		}
		~UniquePtr()
		{
			delete handle;
		}
		T* operator->()
		{
			return handle;
		}
		bool isValid()
		{
			return handle != nullptr;
		}
	private:
		T* handle;
	};
}
