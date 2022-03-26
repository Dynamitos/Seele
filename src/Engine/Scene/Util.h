#pragma once
#include "MinimalEngine.h"

namespace Seele
{
template<typename T>
class Writable
{
public:
    Writable()
    {}
    Writable(T initialData)
        : toBeWritten(initialData)
        , data(initialData)
    {}
    Writable(const Writable& other) = delete;
    Writable(Writable&& other) = default;
    ~Writable() = default;
    Writable& operator=(const Writable& other) const = delete;
    Writable& operator=(Writable&& other) const = delete;
    const Writable& operator=(const T& other) const
    {
        deferWrite(other);
        return *this;
    }
    const Writable& operator=(T&& other) const
    {
        deferWrite(std::move(other));
        return *this;
    }
    template<typename Other>
    const Writable& operator+(Other&& other) const
    {
        deferWrite(data + std::forward<Other>(other));
        return *this;
    }
    template<typename Other>
    const Writable& operator-(Other&& other) const
    {
        deferWrite(data - std::forward<Other>(other));
        return *this;
    }
    template<typename Other>
    const Writable& operator*(Other&& other) const
    {
        deferWrite(data * std::forward<Other>(other));
        return *this;
    }
    template<typename Other>
    const Writable& operator/(Other&& other) const
    {
        deferWrite(data / std::forward<Other>(other));
        return *this;
    }
    template<typename Other>
    const Writable& operator%(Other&& other) const
    {
        deferWrite(data % std::forward<Other>(other));
        return *this;
    }
    template<typename Other>
    const Writable& operator^(Other&& other) const
    {
        deferWrite(data ^ std::forward<Other>(other));
        return *this;
    }
    template<typename Other>
    const Writable& operator&(Other&& other) const
    {
        deferWrite(data & std::forward<Other>(other));
        return *this;
    }
    template<typename Other>
    const Writable& operator|(Other&& other) const
    {
        deferWrite(data | std::forward<Other>(other));
        return *this;
    }
    template<typename Type>
    bool operator==(Type other) const
    {
        return data == other.data;
    }
    template<typename Type>
    bool operator<=>(Type other) const
    {
        return data <=> other.data;
    }
    const Writable& operator++() const
    {
        deferWrite(data+1);
        return *this;
    }
    const Writable& operator--() const
    {
        deferWrite(data-1);
        return *this;
    }
    Writable&& operator++(int) const
    {
        Writable tmp(data);
        ++*this;
        return std::move(tmp);
    }
    Writable&& operator--(int) const
    {
        Writable tmp(data);
        --*this;
        return std::move(tmp);
    }
    const T& operator*() const
    {
        return data;
    }
    const T& get() const
    {
        return data;
    }
    const T& operator->() const
    {
        return data;
    }
    void update()
    {
        // lock should not be necessary, but lets keep it for now
        //std::scoped_lock lock(dataLock);
        data = toBeWritten;
        dirty = false;
    }
private:
    template<typename Type>
    void deferWrite(Type&& newValue) const
    {
        std::scoped_lock lock(dataLock);
        assert(!dirty);
        toBeWritten = std::forward<Type>(newValue);
        dirty = true;
    }
    mutable bool dirty = false;
    mutable std::mutex dataLock;
    mutable T toBeWritten;
    T data;
};
template<typename A>
concept writable = requires(A&& a)
{
    a.update();
};
} // namespace Seele
