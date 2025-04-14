#pragma once
#include "Math/AABB.h"
namespace Seele {

struct PoolRange {
    uint32 offset;
    uint32 size;
};
struct MeshData {
    AABB bounding;
    PoolRange meshletRange;
    // offset into the global index buffer
    PoolRange indicesRange;
};
struct InstanceData {
    Matrix4 transformMatrix;
    Matrix4 inverseTransformMatrix;
};
} // namespace Seele