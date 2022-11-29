#pragma once
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Containers/Array.h"

namespace Seele
{
struct AABB
{
    Math::Vector min = Math::Vector(std::numeric_limits<float>::max());
    Math::Vector max = Math::Vector(std::numeric_limits<float>::min());
    float surfaceArea() const
    {
        Math::Vector d = max - min;
        return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
    }
    bool intersects(const AABB& other) const
    {
        if (min.x > other.max.x
         || max.x < other.min.x)
        {
            return false;
        }
        if (min.y > other.max.y
         || max.y < other.min.y)
        {
            return false;
        }
        if (min.z > other.max.z
         || max.z < other.min.z)
        {
            return false;
        }
        return true;
    }
    bool contains(const AABB& other) const
    {
        if (min.x > other.min.x
         || max.x < other.max.x)
        {
            return false;
        }
        if (min.y > other.min.y
         || max.y < other.max.y)
        {
            return false;
        }
        if (min.z > other.min.z
         || max.z < other.max.z)
        {
            return false;
        }
        return true;
    }
    AABB getTransformedBox(const Math::Matrix4& matrix) const
    {
        StaticArray<Math::Vector, 8> corners;
        corners[0] = Math::Vector(min.x, min.y, min.z);
        corners[1] = Math::Vector(min.x, min.y, max.z);
        corners[2] = Math::Vector(min.x, max.y, min.z);
        corners[3] = Math::Vector(min.x, max.y, max.z);
        corners[4] = Math::Vector(max.x, min.y, min.z);
        corners[5] = Math::Vector(max.x, min.y, max.z);
        corners[6] = Math::Vector(max.x, max.y, min.z);
        corners[7] = Math::Vector(max.x, max.y, max.z);
        Math::Vector tmin = Math::Vector(1, 1, 1) * std::numeric_limits<float>::max();
        Math::Vector tmax = Math::Vector(1, 1, 1) * std::numeric_limits<float>::min();
        for(int i = 0; i < 8; ++i)
        {
            Math::Vector transformed = matrix * Math::Vector4(corners[i], 1.0f);
            tmin = Math::Vector(std::min(tmin.x, transformed.x), std::min(tmin.y, transformed.y), std::min(tmin.z, transformed.z));
            tmax = Math::Vector(std::max(tmax.x, transformed.x), std::max(tmax.y, transformed.y), std::max(tmax.z, transformed.z));
        }
        return AABB {
            .min = tmin,
            .max = tmax,
        };
    }
    void adjust(const Math::Vector vertex)
    {
        min.x = std::min(min.x, vertex.x);
        min.y = std::min(min.y, vertex.y);
        min.z = std::min(min.z, vertex.z);
        
        max.x = std::max(max.x, vertex.x);
        max.y = std::max(max.y, vertex.y);
        max.z = std::max(max.z, vertex.z);
    }
    AABB combine(const AABB& other) const
    {
        return AABB {
            .min = Math::Vector(
                std::min(min.x, other.min.x),
                std::min(min.y, other.min.y),
                std::min(min.z, other.min.z)
            ),
            .max = Math::Vector (
                std::max(max.x, other.max.x),
                std::max(max.y, other.max.y),
                std::max(max.z, other.max.z)
            ),
        };
    }
};
} // namespace Seele