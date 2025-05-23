#pragma once
#include "Graphics/Enums.h"
#include "Math/AABB.h"

namespace Seele {
struct Meshlet {
    AABB boundingBox;
    uint32 uniqueVertices[Gfx::numVerticesPerMeshlet];       // unique vertiex indices in the vertex data
    uint8 primitiveLayout[Gfx::numPrimitivesPerMeshlet * 3]; // indices into the uniqueVertices array, only uint8 needed
    uint32 numVertices;
    uint32 numPrimitives;
    uint32 lod = 0;
};
} // namespace Seele