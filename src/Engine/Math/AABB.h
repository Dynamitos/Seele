#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Containers/Array.h"
#include "Graphics/DebugVertex.h"
namespace Seele
{
struct AABB
{
    Vector min = Vector(std::numeric_limits<float>::max());
    float pad0; // So that it can be used directly in shaders
    Vector max = Vector(std::numeric_limits<float>::lowest());// cause of reasons
    float pad1;
    void visualize(Array<DebugVertex>& vertices) const
    {
        StaticArray<DebugVertex, 8> corners;
        corners[0] = DebugVertex { .position = Vector(min.x, min.y, min.z), .color = Vector(0, 1, 0) };
        corners[1] = DebugVertex { .position = Vector(min.x, min.y, max.z), .color = Vector(0, 1, 0) };
        corners[2] = DebugVertex { .position = Vector(min.x, max.y, min.z), .color = Vector(0, 1, 0) };
        corners[3] = DebugVertex { .position = Vector(min.x, max.y, max.z), .color = Vector(0, 1, 0) };
        corners[4] = DebugVertex { .position = Vector(max.x, min.y, min.z), .color = Vector(0, 1, 0) };
        corners[5] = DebugVertex { .position = Vector(max.x, min.y, max.z), .color = Vector(0, 1, 0) };
        corners[6] = DebugVertex { .position = Vector(max.x, max.y, min.z), .color = Vector(0, 1, 0) };
        corners[7] = DebugVertex { .position = Vector(max.x, max.y, max.z), .color = Vector(0, 1, 0) };

        vertices.add(corners[0]);
        vertices.add(corners[1]);

        vertices.add(corners[1]);
        vertices.add(corners[3]);

        vertices.add(corners[2]);
        vertices.add(corners[3]);

        vertices.add(corners[0]);
        vertices.add(corners[2]);
        
        vertices.add(corners[0]);
        vertices.add(corners[4]);

        vertices.add(corners[1]);
        vertices.add(corners[5]);

        vertices.add(corners[2]);
        vertices.add(corners[6]);

        vertices.add(corners[3]);
        vertices.add(corners[7]);
        
        vertices.add(corners[4]);
        vertices.add(corners[5]);

        vertices.add(corners[5]);
        vertices.add(corners[7]);

        vertices.add(corners[6]);
        vertices.add(corners[7]);

        vertices.add(corners[4]);
        vertices.add(corners[6]);
    }
    float surfaceArea() const
    {
        Vector d = max - min;
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
    AABB getTransformedBox(const Matrix4& matrix) const
    {
        StaticArray<Vector, 8> corners;
        corners[0] = Vector(min.x, min.y, min.z);
        corners[1] = Vector(min.x, min.y, max.z);
        corners[2] = Vector(min.x, max.y, min.z);
        corners[3] = Vector(min.x, max.y, max.z);
        corners[4] = Vector(max.x, min.y, min.z);
        corners[5] = Vector(max.x, min.y, max.z);
        corners[6] = Vector(max.x, max.y, min.z);
        corners[7] = Vector(max.x, max.y, max.z);
        Vector tmin = Vector(1, 1, 1) * std::numeric_limits<float>::max();
        Vector tmax = Vector(1, 1, 1) * std::numeric_limits<float>::lowest();
        for(int i = 0; i < 8; ++i)
        {
            Vector transformed = matrix * Vector4(corners[i], 1.0f);
            tmin = Vector(std::min(tmin.x, transformed.x), std::min(tmin.y, transformed.y), std::min(tmin.z, transformed.z));
            tmax = Vector(std::max(tmax.x, transformed.x), std::max(tmax.y, transformed.y), std::max(tmax.z, transformed.z));
        }
        return AABB {
            .min = tmin,
            .max = tmax,
        };
    }
    void adjust(const Vector vertex)
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
            .min = Vector(
                std::min(min.x, other.min.x),
                std::min(min.y, other.min.y),
                std::min(min.z, other.min.z)
            ),
            .max = Vector (
                std::max(max.x, other.max.x),
                std::max(max.y, other.max.y),
                std::max(max.z, other.max.z)
            ),
        };
    }
};
} // namespace Seele