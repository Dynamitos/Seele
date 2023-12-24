#include "Meshlet.h"
#include "Containers/Map.h"
#include "Containers/List.h"
#include "Containers/Set.h"

using namespace Seele;

void Meshlet::build(const Array<Vector>& positions, const Array<uint32>& indices, Array<Meshlet>& meshlets)
{
    Meshlet current = {
        .numVertices = 0,
        .numPrimitives = 0,
    };
    auto findIndex = [&current](uint32 index) -> int {
        for (uint32 i = 0; i < current.numVertices; ++i)
        {
            if (current.uniqueVertices[i] == index)
            {
                return i;
            }
        }
        if (current.numVertices == Gfx::numVerticesPerMeshlet)
        {
            return -1;
        }
        current.uniqueVertices[current.numVertices] = index;
        return current.numVertices++;
        };
    auto completeMeshlet = [&positions, &meshlets, &current]() {
        for (uint32 i = 0; i < current.numVertices; ++i)
        {
            current.boundingBox.adjust(positions[current.uniqueVertices[i]]);
        }
        meshlets.add(current);
        current = {
            .numVertices = 0,
            .numPrimitives = 0,
        };
        };
    for (size_t faceIndex = 0; faceIndex < indices.size() / 3; ++faceIndex)
    {
        int f1 = findIndex(indices[faceIndex * 3 + 0]);
        int f2 = findIndex(indices[faceIndex * 3 + 1]);
        int f3 = findIndex(indices[faceIndex * 3 + 2]);

        if (f1 == -1 || f2 == -1 || f3 == -1)
        {
            completeMeshlet();
            f1 = findIndex(indices[faceIndex * 3 + 0]);
            f2 = findIndex(indices[faceIndex * 3 + 1]);
            f3 = findIndex(indices[faceIndex * 3 + 2]);
        }
        current.primitiveLayout[current.numPrimitives * 3 + 0] = uint8(f1);
        current.primitiveLayout[current.numPrimitives * 3 + 1] = uint8(f2);
        current.primitiveLayout[current.numPrimitives * 3 + 2] = uint8(f3);
        current.numPrimitives++;
        if (current.numPrimitives == Gfx::numPrimitivesPerMeshlet)
        {
            completeMeshlet();
        }
    }
    if (current.numVertices > 0)
    {
        completeMeshlet();
    }
}
