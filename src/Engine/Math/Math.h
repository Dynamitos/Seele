#include "Vector.h"
#include "Matrix.h"

namespace Seele
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
struct Rect3D
{
	Vector3 size;
	Vector3 offset;
};
} // namespace Seele