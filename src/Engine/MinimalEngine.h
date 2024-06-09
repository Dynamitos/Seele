#pragma once
#include "EngineTypes.h"
#include <assert.h>
#include <cstddef>
#include <memory>
#include <utility>


#define DEFINE_REF(x)                                                                                                                      \
    typedef ::Seele::RefPtr<x> P##x;                                                                                                       \
    typedef ::Seele::UniquePtr<x> UP##x;                                                                                                   \
    typedef ::Seele::OwningPtr<x> O##x;

#define DECLARE_REF(x)                                                                                                                     \
    class x;                                                                                                                               \
    typedef ::Seele::RefPtr<x> P##x;                                                                                                       \
    typedef ::Seele::UniquePtr<x> UP##x;                                                                                                   \
    typedef ::Seele::OwningPtr<x> O##x;

#define DECLARE_NAME_REF(nmsp, x)                                                                                                          \
    namespace nmsp {                                                                                                                       \
    class x;                                                                                                                               \
    typedef RefPtr<x> P##x;                                                                                                                \
    typedef UniquePtr<x> UP##x;                                                                                                            \
    typedef OwningPtr<x> O##x;                                                                                                             \
    }

namespace Seele {
template <typename T> class RefPtr {
  public:
    constexpr RefPtr() noexcept : object(nullptr) {}
    constexpr RefPtr(std::nullptr_t) noexcept : object(nullptr) {}
    RefPtr(T* ptr) : object(ptr) {}
    constexpr RefPtr(const RefPtr& other) noexcept : object(other.object) {}
    constexpr RefPtr(RefPtr&& rhs) noexcept : object(std::move(rhs.object)) { rhs.object = nullptr; }
    template <typename F> constexpr RefPtr<F> cast() {
        T* t = object;
        F* f = dynamic_cast<F*>(t);
        if (f == nullptr) {
            return nullptr;
        }
        return RefPtr<F>(f);
    }
    template <typename F> constexpr const RefPtr<F> cast() const {
        T* t = object;
        F* f = dynamic_cast<F*>(t);
        if (f == nullptr) {
            return nullptr;
        }
        return RefPtr<F>(f);
    }
    constexpr RefPtr& operator=(const RefPtr& other) {
        if (this != &other) {
            object = other.object;
        }
        return *this;
    }
    constexpr RefPtr& operator=(RefPtr&& rhs) noexcept {
        if (this != &rhs) {
            object = std::move(rhs.object);
            rhs.object = nullptr;
        }
        return *this;
    }
    constexpr ~RefPtr() {}
    constexpr bool operator==(const RefPtr& rhs) const noexcept { return object == rhs.object; }
    constexpr auto operator<=>(const RefPtr& rhs) const noexcept { return object <=> rhs.object; }
    template <typename F> constexpr bool operator==(const RefPtr<F>& rhs) const noexcept { return object == rhs.getHandle(); }
    template <typename F> constexpr auto operator<=>(const RefPtr<F>& rhs) const noexcept { return object <=> rhs.getHandle(); }
    template <typename F> constexpr operator RefPtr<F>() { return RefPtr<F>(static_cast<F*>(object)); }
    template <typename F> constexpr operator RefPtr<F>() const { return RefPtr<F>(static_cast<F*>(object)); }
    constexpr T* operator->() { return object; }
    constexpr const T* operator->() const { return object; }
    constexpr T* getHandle() { return object; }
    constexpr const T* getHandle() const { return object; }

  private:
    T* object;
};
template <typename T, typename Deleter = std::default_delete<T>> class OwningPtr {
  public:
    OwningPtr() : pointer(nullptr) {}
    OwningPtr(T* ptr) : pointer(ptr) {}
    template <typename F> OwningPtr(OwningPtr<F>&& other) {
        pointer = dynamic_cast<T*>(*other);
        other.clear();
    }
    OwningPtr(const OwningPtr& other) = delete;
    OwningPtr(OwningPtr&& other) noexcept {
        pointer = other.pointer;
        other.pointer = nullptr;
    }
    OwningPtr& operator=(const OwningPtr& other) = delete;
    OwningPtr& operator=(OwningPtr&& other) noexcept {
        if (this != &other) {
            if (pointer != nullptr) {
                Deleter()(pointer);
            }
            pointer = other.pointer;
            other.pointer = nullptr;
        }
        return *this;
    }
    ~OwningPtr() { Deleter()(pointer); }
    operator RefPtr<T>() { return RefPtr<T>(pointer); }
    operator RefPtr<T>() const { return RefPtr<T>(pointer); }
    constexpr T* operator->() { return pointer; }
    constexpr const T* operator->() const { return pointer; }
    constexpr T* operator*() { return pointer; }
    constexpr const T* operator*() const { return pointer; }
    constexpr bool operator==(std::nullptr_t) const noexcept { return pointer == nullptr; }
    constexpr auto operator<=>(const OwningPtr& rhs) const noexcept { return pointer <=> rhs.pointer; }
    // INTERNAL USE
    constexpr void clear() { pointer = nullptr; }

  private:
    friend class RefPtr<T>;
    T* pointer;
};
template <typename T> class UniquePtr {
  public:
    UniquePtr() : handle(nullptr) {}
    UniquePtr(std::nullptr_t) : handle(nullptr) {}
    UniquePtr(T* ptr) : handle(ptr) {}
    UniquePtr(const UniquePtr& rhs) = delete;
    UniquePtr(UniquePtr&& rhs) noexcept : handle(rhs.handle) { rhs.handle = nullptr; }
    UniquePtr& operator=(const UniquePtr& rhs) = delete;
    UniquePtr& operator=(UniquePtr&& rhs) {
        if (this != &rhs) {
            handle = rhs.handle;
            rhs.handle = nullptr;
        }
        return *this;
    }
    ~UniquePtr() { delete handle; }
    inline bool operator==(const UniquePtr& other) const { return handle == other.handle; }
    inline bool operator!=(const UniquePtr& other) const { return handle != other.handle; }
    inline T* operator->() { return handle; }
    inline T* getHandle() { return handle; }
    bool isValid() { return handle != nullptr; }

  private:
    T* handle;
};
} // namespace Seele
