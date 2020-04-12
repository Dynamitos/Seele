#pragma once
#include <assert.h>
#include <memory>
#include <atomic>
#include <cstring>
#include <iostream>
#include "Containers/Map.h"
#include "EngineTypes.h"
#include "Math/Math.h"

#define DEFINE_REF(x) typedef RefPtr<x> P##x; \
		typedef UniquePtr<x> UP##x; \
		typedef WeakPtr<x> W##x;

#define DECLARE_REF(x) class x; \
		typedef RefPtr<x> P##x; \
		typedef UniquePtr<x> UP##x; \
		typedef WeakPtr<x> W##x;

extern Seele::Map<void*, void*> registeredObjects;
namespace Seele
{
	template<typename T>
	class RefObject
	{
	public:
		RefObject(T* ptr)
			: handle(ptr)
			, refCount(1)
		{
			registeredObjects[ptr] = this;
		}
		RefObject(const RefObject& rhs)
			: handle(rhs.handle)
			, refCount(rhs.refCount)
		{
		}
		RefObject(RefObject&& rhs)
			: handle(std::move(rhs.handle))
			, refCount(std::move(rhs.refCount))
		{}
		~RefObject()
		{
			registeredObjects.erase(handle);
			delete handle;
		}
		RefObject& operator=(const RefObject& rhs)
		{
			if(*this != rhs)
			{
				handle = rhs.handle;
				refCount = rhs.refCount;
			}
			return *this;
		}
		RefObject& operator=(RefObject&& rhs)
		{
			if(*this != rhs)
			{
				handle = std::move(rhs.handle);
				refCount = std::move(rhs.refCount);
				rhs.handle = nullptr;
				rhs.refCount = 0;
			}
			return *this;
		}
		bool operator==(const RefObject& other) const
		{
			return handle == other.handle;
		}
		bool operator!=(const RefObject& other) const
		{
			return handle != other.handle;
		}
		bool operator<(const RefObject& other) const
		{
			return handle < other.handle;
		}
		void addRef()
		{
			refCount++;
		}
		void removeRef()
		{
			refCount--;
			if (refCount.load() == 0)
			{
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


	template<typename T>
	class RefPtr
	{
	public:
		RefPtr()
		{
			object = nullptr;
		}
		RefPtr(nullptr_t)
		{
			object = nullptr;
		}
		RefPtr(T* ptr)
		{
			auto registeredObj = registeredObjects.find(ptr);
			if(registeredObj == registeredObjects.end())
			{
				object = new RefObject<T>(ptr);
			}
			else
			{
				object = (RefObject<T>*)registeredObj->value;
			}
		}
		explicit RefPtr(RefObject<T>* other)
			: object(other)
		{
			object->addRef();
		}
		RefPtr(const RefPtr& other)
			: object(other.object)
		{
			if(object != nullptr)
			{
				object->addRef();
			}
		}
		RefPtr(RefPtr&& rhs)
			: object(std::move(rhs.object))
		{
			rhs.object = nullptr;
			//Dont change references, they stay the same
		}
		template<typename F>
		RefPtr(const RefPtr<F>& other)
		{
			F* f = other.getObject()->getHandle();
			static_cast<T*>(f);
			object = (RefObject<T>*)other.getObject();
			object->addRef();
		}

		template<typename F>
		RefPtr<F> cast()
		{
			T* t = object->getHandle();
			F* f = dynamic_cast<F*>(t);
			if (f == nullptr)
			{
				return nullptr;
			}
			RefObject<F>* newObject = (RefObject<F>*)object;
			return RefPtr<F>(newObject);
		}

		template<typename F>
		const RefPtr<F> cast() const
		{
			T* t = object->getHandle();
			F* f = dynamic_cast<F*>(t);
			if (f == nullptr)
			{
				return nullptr;
			}
			RefObject<F>* newObject = (RefObject<F>*)object;
			return RefPtr<F>(newObject);
		}

		RefPtr& operator=(const RefPtr& other)
		{
			if (*this != other)
			{
				if (object != nullptr)
				{
					object->removeRef();
				}
				object = other.object;
				if(object != nullptr)
				{
					object->addRef();
				}
			}
			return *this;
		}
		RefPtr& operator=(RefPtr&& rhs)
		{
			if(*this != rhs)
			{
				if(object != nullptr)
				{
					object->removeRef();
				}
				object = std::move(rhs.object);
				rhs.object = nullptr;
			}
			return *this;
		}
		~RefPtr()
		{
			if(object != nullptr)
			{
				object->removeRef();
			}
		}
		bool operator==(const RefPtr& other) const
		{
			return object == other.object;
		}
		bool operator!=(const RefPtr& other) const
		{
			return object != other.object;
		}
		T* operator->()
		{
			assert(object != nullptr);
			return object->getHandle();
		}
		const T* operator->() const
		{
			assert(object != nullptr);
			return object->getHandle();
		}
		RefObject<T>* getObject() const
		{
			return object;
		}
		T* getHandle()
		{
			return object->getHandle();
		}
		const T* getHandle() const
		{
			return object->getHandle();
		}
	private:
		RefObject<T>* object;
	};
	//A weak pointer has no ownership over an object and thus cant delete it
	template<typename T>
	class WeakPtr
	{
	public:
		WeakPtr()
			: pointer(nullptr)
		{}
		WeakPtr(RefPtr<T>& sharedPtr)
			: pointer(sharedPtr)
		{}
		WeakPtr& operator=(WeakPtr<T>& weakPtr)
		{
			pointer = weakPtr.pointer;
			return *this;
		}
		WeakPtr& operator=(RefPtr<T>& sharedPtr)
		{
			pointer = sharedPtr;
			return *this;
		}
	private:
		RefPtr<T> pointer;
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

using namespace Seele;