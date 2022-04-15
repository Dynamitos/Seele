#pragma once
#include "Containers/Map.h"
#include "EngineTypes.h"
#include "Math/Math.h"
#include <map>

#define DEFINE_REF(x)           \
    typedef RefPtr<x> P##x;     \
    typedef UniquePtr<x> UP##x; \
    typedef WeakPtr<x> W##x;

#define DECLARE_REF(x)          \
    class x;                    \
    typedef RefPtr<x> P##x;     \
    typedef UniquePtr<x> UP##x; \
    typedef WeakPtr<x> W##x;


#define DECLARE_NAME_REF(nmsp, x)   \
    namespace nmsp                  \
    {                               \
        class x;                    \
        typedef RefPtr<x> P##x;     \
        typedef UniquePtr<x> UP##x; \
        typedef WeakPtr<x> W##x;    \
    }

extern std::map<void *, void *> registeredObjects;
extern std::mutex registeredObjectsLock;
namespace Seele
{
template <typename T, typename Deleter>
class RefPtr;
template <typename T, typename Deleter>
class RefObject
{
public:
    RefObject(T *ptr, Deleter&& deleter)
        : handle(ptr)
        , deleter(std::move(deleter))
        , refCount(1)
    {
    }
    RefObject(const RefObject &rhs) = delete;
    RefObject(RefObject &&rhs)
        : handle(std::move(rhs.handle)), refCount(std::move(rhs.refCount))
    {
    }
    ~RefObject()
    {
        {
            std::scoped_lock lock(registeredObjectsLock);
            registeredObjects.erase(handle);
        }
//    #pragma warning( disable: 4150)
        deleter(handle);
        handle = nullptr;
//    #pragma warning( default: 4150)
    }
    RefObject &operator=(const RefObject &rhs) = delete;
    RefObject &operator=(RefObject &&rhs)
    {
        if (*this != rhs)
        {
            handle = std::move(rhs.handle);
            refCount = std::move(rhs.refCount);
            rhs.handle = nullptr;
            rhs.refCount = 0;
        }
        return *this;
    }
    bool operator==(const RefObject &rhs) const 
    {
        return handle == rhs.handle;
    }
    auto operator<=>(const RefObject& rhs) const
    {
        return handle <=> rhs.handle;
    }
    void addRef()
    {
        refCount++;
    }
    void removeRef()
    {
        refCount--;
        if (refCount == 0)
        {
            delete this;
        }
    }
    T *getHandle() const
    {
        return handle;
    }
private:
    T *handle;
    Deleter deleter;
    std::atomic_uint64_t refCount;
    friend class RefPtr<T, Deleter>;
};

template <typename T, typename Deleter = std::default_delete<T>>
class RefPtr
{
public:
    constexpr RefPtr() noexcept
    {
        object = nullptr;
    }
    constexpr RefPtr(nullptr_t) noexcept
    {
        object = nullptr;
    }
    constexpr RefPtr(T *ptr, Deleter deleter = Deleter())
    {
        std::scoped_lock l(registeredObjectsLock);
        auto registeredObj = registeredObjects.find(ptr);
        // get here for thread safetly
        auto registeredEnd = registeredObjects.end();
        if (registeredObj == registeredEnd)
        {
            object = new RefObject<T, Deleter>(ptr, std::move(deleter));
            registeredObjects[ptr] = object;
        }
        else
        {		
            object = (RefObject<T, Deleter> *)registeredObj->second;
            object->addRef();
        }
    }
    constexpr explicit RefPtr(RefObject<T, Deleter> *other) noexcept
        : object(other)
    {
        if(object != nullptr)
        {
            object->addRef();
        }
    }
    constexpr RefPtr(const RefPtr &other) noexcept
        : object(other.object)
    {
        if (object != nullptr)
        {
            object->addRef();
        }
    }
    constexpr RefPtr(RefPtr &&rhs) noexcept
        : object(std::move(rhs.object))
    {
        rhs.object = nullptr;
        //Dont change references, they stay the same
    }
    template <typename F>
    constexpr RefPtr(const RefPtr<F> &other)
    {
        if(other == nullptr)
        {
            return;
        }
        F *f = other.getObject()->getHandle();
        assert(static_cast<T *>(f));
        object = (RefObject<T, Deleter> *)other.getObject();
        object->addRef();
    }

    template <typename F, typename DeleterF = std::default_delete<F>>
    constexpr RefPtr<F, DeleterF> cast()
    {
        T *t = object->getHandle();
        F *f = dynamic_cast<F *>(t);
        if (f == nullptr)
        {
            return nullptr;
        }
        RefObject<F, DeleterF> *newObject = (RefObject<F, DeleterF> *)object;
        return RefPtr<F, DeleterF>(newObject);
    }

    template <typename F, typename DeleterF = std::default_delete<F>>
    constexpr const RefPtr<F, DeleterF> cast() const
    {
        T *t = object->getHandle();
        F *f = dynamic_cast<F *>(t);
        if (f == nullptr)
        {
            return nullptr;
        }
        RefObject<F, DeleterF> *newObject = (RefObject<F, DeleterF> *)object;
        return RefPtr<F, DeleterF>(newObject);
    }

    constexpr RefPtr &operator=(const RefPtr &other)
    {
        if (this != &other)
        {
            if (object != nullptr)
            {
                object->removeRef();
            }
            object = other.object;
            if (object != nullptr)
            {
                object->addRef();
            }
        }
        return *this;
    }
    constexpr RefPtr &operator=(RefPtr &&rhs)
    {
        if (this != &rhs)
        {
            if (object != nullptr)
            {
                object->removeRef();
            }
            object = std::move(rhs.object);
            rhs.object = nullptr;
        }
        return *this;
    }
    constexpr ~RefPtr()
    {
        if (object != nullptr)
        {
            object->removeRef();
        }
    }
    constexpr bool operator==(const RefPtr& rhs) const noexcept
    {
        return object == rhs.object;
    }
    constexpr auto operator<=>(const RefPtr &rhs) const noexcept
    {
        return object <=> rhs.object;
    }
    constexpr T *operator->()
    {
        assert(object != nullptr);
        return object->handle;
    }
    constexpr const T *operator->() const
    {
        assert(object != nullptr);
        return object->handle;
    }
    constexpr RefObject<T, Deleter> *getObject() const noexcept
    {
        return object;
    }
    constexpr T *getHandle()
    {
        return object->getHandle();
    }
    constexpr const T *getHandle() const
    {
        return object->getHandle();
    }
    constexpr RefPtr<T, Deleter> clone()
    {
        return RefPtr<T, Deleter>(new T(*getHandle()));
    }

private:
    RefObject<T, Deleter> *object;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar & *object->getHandle();
    }
};
template <typename T>
class UniquePtr
{
public:
    UniquePtr()
        : handle(nullptr)
    {
    }
    UniquePtr(nullptr_t)
        : handle(nullptr)
    {
    }
    UniquePtr(T *ptr)
        : handle(ptr)
    {
    }
    UniquePtr(const UniquePtr &rhs) = delete;
    UniquePtr(UniquePtr &&rhs) noexcept
        : handle(rhs.handle)
    {
        rhs.handle = nullptr;
    }
    UniquePtr &operator=(const UniquePtr &rhs) = delete;
    UniquePtr &operator=(UniquePtr &&rhs)
    {
        handle = rhs.handle;
        rhs.handle = nullptr;
        return *this;
    }
    ~UniquePtr()
    {
        delete handle;
    }
    inline bool operator==(const UniquePtr &other) const
    {
        return handle == other.handle;
    }
    inline bool operator!=(const UniquePtr &other) const
    {
        return handle != other.handle;
    }
    inline T *operator->()
    {
        return handle;
    }
    inline T* getHandle()
    {
        return handle;
    }
    bool isValid()
    {
        return handle != nullptr;
    }

private:
    T *handle;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & *handle;
    }
};
//A weak pointer has no ownership over an object and thus cant delete it
template <typename T>
class WeakPtr
{
public:
    WeakPtr()
        : pointer(nullptr)
    {
    }
    WeakPtr(RefPtr<T> &sharedPtr)
        : pointer(sharedPtr)
    {
    }
    WeakPtr &operator=(WeakPtr<T> &weakPtr)
    {
        pointer = weakPtr.pointer;
        return *this;
    }
    WeakPtr &operator=(RefPtr<T> &sharedPtr)
    {
        pointer = sharedPtr;
        return *this;
    }

private:
    RefPtr<T> pointer;
};
} // namespace Seele
