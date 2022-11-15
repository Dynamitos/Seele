#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "EngineTypes.h"

namespace Seele
{
namespace Math
{
struct Rect
{
	Rect()
		: size(0, 0), offset(0, 0)
	{
	}
	Rect(float sizeX, float sizeY, float offsetX, float offsetY)
		: size(sizeX, sizeY), offset(offsetX, offsetY)
	{
	}
	Rect(Vector2 size, Vector2 offset)
		: size(size), offset(offset)
	{
	}
	bool isEmpty() const
	{
		return size.x == 0 || size.y == 0;
	}
	Vector2 size;
	Vector2 offset;
};
//Unsigned int
struct URect
{
	URect()
		: size(0, 0), offset(0, 0)
	{
	}
	URect(uint32 sizeX, uint32 sizeY, uint32 offsetX, uint32 offsetY)
		: size(sizeX, sizeY), offset(offsetX, offsetY)
	{
	}
	URect(UVector2 size, UVector2 offset)
		: size(size), offset(offset)
	{
	}
	bool isEmpty() const
	{
		return size.x == 0 || size.y == 0;
	}
	UVector2 size;
	UVector2 offset;
};
struct Rect3D
{
	Vector size;
	Vector offset;
};
template <typename T>
inline constexpr T align(const T ptr, int64_t alignment)
{
	return (T)(((uint64_t)ptr + alignment - 1) & ~(alignment - 1));
}
} // namespace Math
} // namespace Seele