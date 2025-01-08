#pragma once
#include "ArchiveBuffer.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"


namespace Seele {
namespace Serialization {
template <std::integral T> static void save(ArchiveBuffer& buffer, const T& type) { buffer.writeBytes(&type, sizeof(type)); }
template <std::integral T> static void load(ArchiveBuffer& buffer, T& type) { buffer.readBytes(&type, sizeof(type)); }
template <std::floating_point T> static void save(ArchiveBuffer& buffer, const T& type) { buffer.writeBytes(&type, sizeof(type)); }
template <std::floating_point T> static void load(ArchiveBuffer& buffer, T& type) { buffer.readBytes(&type, sizeof(type)); }
static void save(ArchiveBuffer& buffer, const UVector2& vec) {
    save(buffer, vec.x);
    save(buffer, vec.y);
}
static void load(ArchiveBuffer& buffer, UVector2& vec) {
    load(buffer, vec.x);
    load(buffer, vec.y);
}
static void save(ArchiveBuffer& buffer, const IVector2& vec) {
    save(buffer, vec.x);
    save(buffer, vec.y);
}
static void load(ArchiveBuffer& buffer, IVector2& vec) {
    load(buffer, vec.x);
    load(buffer, vec.y);
}
static void save(ArchiveBuffer& buffer, const Vector2& vec) {
    save(buffer, vec.x);
    save(buffer, vec.y);
}
static void load(ArchiveBuffer& buffer, Vector2& vec) {
    load(buffer, vec.x);
    load(buffer, vec.y);
}
static void save(ArchiveBuffer& buffer, const Vector& vec) {
    save(buffer, vec.x);
    save(buffer, vec.y);
    save(buffer, vec.z);
}
static void load(ArchiveBuffer& buffer, Matrix4& mat) { buffer.readBytes(&mat, sizeof(Matrix4)); }
static void save(ArchiveBuffer& buffer, const Matrix4& mat) { buffer.writeBytes(&mat, sizeof(Matrix4)); }
static void load(ArchiveBuffer& buffer, Vector& vec) {
    load(buffer, vec.x);
    load(buffer, vec.y);
    load(buffer, vec.z);
}
template <enumeration T> static void save(ArchiveBuffer& buffer, const T& type) { buffer.writeBytes(&type, sizeof(type)); }
template <enumeration T> static void load(ArchiveBuffer& buffer, T& type) { buffer.readBytes(&type, sizeof(type)); }
template <typename T>
static void save(ArchiveBuffer& buffer, const T& type)
    requires(serializable<ArchiveBuffer, T>)
{
    type.save(buffer);
}
template <typename T>
static void load(ArchiveBuffer& buffer, T& type)
    requires(serializable<ArchiveBuffer, T>)
{
    type.load(buffer);
}
template <typename T> static void save(ArchiveBuffer& buffer, const OwningPtr<T>& ptr);
template <typename T> static void load(ArchiveBuffer& buffer, OwningPtr<T>& ptr);
static void save(ArchiveBuffer& buffer, const std::string& type) {
    uint64 length = type.size();
    buffer.writeBytes(&length, sizeof(uint64));
    buffer.writeBytes(type.data(), type.size() * sizeof(char));
}
static void load(ArchiveBuffer& buffer, std::string& type) {
    uint64 length = 0;
    buffer.readBytes(&length, sizeof(uint64));
    type.resize(length);
    buffer.readBytes(type.data(), length * sizeof(char));
}
template <typename T>
static void save(ArchiveBuffer& buffer, const Array<T>& type)
    requires(std::is_trivially_copyable_v<T>)
{
    uint64 length = type.size();
    buffer.writeBytes(&length, sizeof(uint64));
    buffer.writeBytes(type.data(), sizeof(T) * type.size());
}
template <typename T>
static void load(ArchiveBuffer& buffer, Array<T>& type)
    requires(std::is_trivially_copyable_v<T>)
{
    uint64 length = 0;
    buffer.readBytes(&length, sizeof(uint64));
    type.resize(length);
    buffer.readBytes(type.data(), sizeof(T) * type.size());
}
template <typename T, size_t N>
static void save(ArchiveBuffer& buffer, const StaticArray<T, N>& type)
    requires(std::is_trivially_copyable_v<T>)
{
    buffer.writeBytes(type.data(), sizeof(T) * N);
}
template <typename T, size_t N>
static void load(ArchiveBuffer& buffer, StaticArray<T, N>& type)
    requires(std::is_trivially_copyable_v<T>)
{
    buffer.readBytes(type.data(), sizeof(T) * N);
}
template <typename T> static void save(ArchiveBuffer& buffer, const Array<T>& type) {
    uint64 length = type.size();
    buffer.writeBytes(&length, sizeof(uint64));
    for (const T& x : type) {
        save(buffer, x);
    }
}
template <typename T> static void load(ArchiveBuffer& buffer, Array<T>& type) {
    uint64 length = 0;
    buffer.readBytes(&length, sizeof(uint64));
    type.reserve(length);
    for (uint32 i = 0; i < length; ++i) {
        T t = T();
        load(buffer, t);
        type.add(std::move(t));
    }
}
} // namespace Serialization
} // namespace Seele