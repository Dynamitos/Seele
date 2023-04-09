#pragma once
#include "MinimalEngine.h"
#include "Containers/Array.h"
#include "Concepts.h"
#include <string>

namespace Seele
{
DECLARE_NAME_REF(Gfx, Graphics)
class ArchiveBuffer
{
public:
    ArchiveBuffer();
    ArchiveBuffer(Gfx::PGraphics graphics);
    void writeBytes(const void* data, uint64 size);
    void readBytes(void* dest, uint64 size);
    void writeToStream(std::ostream& stream);
    void readFromStream(std::istream& stream);
    enum class SeekOp
    {
        CURRENT,
        BEGIN,
        END,
    };
    void seek(int64 s, SeekOp op);
    bool eof() const;
    size_t size() const;
    void rewind();
    Gfx::PGraphics& getGraphics();
private:
    Gfx::PGraphics graphics;
    uint64 version = 0;
    uint64 position = 0;
    Array<uint8> memory;
};
namespace Serialization
{
    template<std::integral T>
    static void save(ArchiveBuffer& buffer, const T& type)
    {
        buffer.writeBytes(&type, sizeof(type));
    }
    template<std::integral T>
    static void load(ArchiveBuffer& buffer, T& type)
    {
        buffer.readBytes(&type, sizeof(type));
    }
    template<std::floating_point T>
    static void save(ArchiveBuffer& buffer, const T& type)
    {
        buffer.writeBytes(&type, sizeof(type));
    }
    template<std::floating_point T>
    static void load(ArchiveBuffer& buffer, T& type)
    {
        buffer.readBytes(&type, sizeof(type));
    }
    template<enumeration T>
    static void save(ArchiveBuffer& buffer, const T& type)
    {
        buffer.writeBytes(&type, sizeof(type));
    }
    template<enumeration T>
    static void load(ArchiveBuffer& buffer, T& type)
    {
        buffer.readBytes(&type, sizeof(type));
    }
    template<typename T>
    static void save2(ArchiveBuffer& buffer, const T& type) requires(serializable<T>)
    {
        type.save(buffer);
    }
    template<typename T>
    static void load2(ArchiveBuffer& buffer, T& type) requires(serializable<T>)
    {
        type.load(buffer);
    }
    //template<class T>
    //static void save(ArchiveBuffer& buffer, const T& type)
    //{
    //    type->save(buffer);
    //}
    //template<class T>
    //static void load(ArchiveBuffer& buffer, T& type)
    //{
    //    type->load(buffer);
    //}
    static void save(ArchiveBuffer& buffer, const std::string& type)
    {
        uint64 length = type.size();
        buffer.writeBytes(&length, sizeof(uint64));
        buffer.writeBytes(type.data(), type.size() * sizeof(char));
    }
    static void load(ArchiveBuffer& buffer, std::string& type)
    {
        uint64 length = 0;
        buffer.readBytes(&length, sizeof(uint64));
        type.resize(length);
        buffer.readBytes(type.data(), length * sizeof(char));
    }
    //template<typename T>
    //static void save(ArchiveBuffer& buffer, const RefPtr<T>& ptr)
    //{
    //    Serialization::save(buffer, *ptr.getHandle());
    //}
    //template<typename T>
    //static void load(ArchiveBuffer& buffer, RefPtr<T>& ptr)
    //{
    //    Serialization::load(buffer, *ptr.getHandle());
    //}
    template<std::integral T>
    static void save(ArchiveBuffer& buffer, const Array<T>& type)
    {
        uint64 length = type.size();
        buffer.writeBytes(&length, sizeof(uint64));
        buffer.writeBytes(type.data(), sizeof(T) * type.size());
    }
    template<std::integral T>
    static void load(ArchiveBuffer& buffer, Array<T>& type)
    {
        uint64 length = 0;
        buffer.readBytes(&length, sizeof(uint64));
        type.resize(length);
        buffer.readBytes(type.data(), sizeof(T) * type.size());
    }
    template<std::floating_point T>
    static void save(ArchiveBuffer& buffer, const Array<T>& type)
    {
        uint64 length = type.size();
        buffer.writeBytes(&length, sizeof(uint64));
        buffer.writeBytes(type.data(), sizeof(T) * type.size());
    }
    template<std::floating_point T>
    static void load(ArchiveBuffer& buffer, Array<T>& type)
    {
        uint64 length = 0;
        buffer.readBytes(&length, sizeof(uint64));
        type.resize(length);
        buffer.readBytes(type.data(), sizeof(T) * type.size());
    }
    template<std::integral T, size_t N>
    static void save(ArchiveBuffer& buffer, const StaticArray<T, N>& type)
    {
        buffer.writeBytes(type.data(), sizeof(T) * N);
    }
    template<std::integral T, size_t N>
    static void load(ArchiveBuffer& buffer, StaticArray<T, N>& type)
    {
        buffer.readBytes(type.data(), sizeof(T) * N);
    }
    template<std::floating_point T, size_t N>
    static void save(ArchiveBuffer& buffer, const StaticArray<T, N>& type)
    {
        buffer.writeBytes(type.data(), sizeof(T) * N);
    }
    template<std::floating_point T, size_t N>
    static void load(ArchiveBuffer& buffer, StaticArray<T, N>& type)
    {
        buffer.readBytes(type.data(), sizeof(T) * N);
    }
    template<typename T>
    static void save(ArchiveBuffer& buffer, const Array<T>& type) requires (!std::is_integral_v<T> && !std::is_floating_point_v<T>)
    {
        uint64 length = type.size();
        buffer.writeBytes(&length, sizeof(uint64));
        for (const auto& x : type)
        {
            save(buffer, x);
        }
    }
    template<typename T>
    static void load(ArchiveBuffer& buffer, Array<T>& type)
    {
        uint64 length = 0;
        buffer.readBytes(&length, sizeof(uint64));
        type.resize(length);
        for (auto& x : type)
        {
            load(buffer, x);
        }
    }
} // namespace Serialization
} // namespace Seele