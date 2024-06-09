#pragma once
#include "Math/AABB.h"
namespace Seele
{
struct MeshData
{
    AABB bounding;
    uint32 numMeshlets = 0;
    uint32 meshletOffset = 0;
    uint32 firstIndex = 0;
    uint32 numIndices = 0;
};
struct InstanceData
{
    Matrix4 transformMatrix;
    Matrix4 inverseTransformMatrix;
};
}