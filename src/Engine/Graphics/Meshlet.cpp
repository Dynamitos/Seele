#include "Meshlet.h"
#include "Containers/Map.h"
#include "Containers/List.h"
#include "Containers/Set.h"

using namespace Seele;

struct Triangle
{
    StaticArray<uint32, 3> indices;
};

int findIndex(Meshlet& current, uint32 index)
{
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
}

void completeMeshlet(Array<Meshlet>& meshlets, Meshlet& current)
{
    meshlets.add(current);
    current = {
        .boundingBox = AABB(),
        .numVertices = 0,
        .numPrimitives = 0,
    };
}

void addTriangle(Array<Meshlet>& meshlets, Meshlet& current, Triangle tri)
{
    int f1 = findIndex(current, tri.indices[0]);
    int f2 = findIndex(current, tri.indices[1]);
    int f3 = findIndex(current, tri.indices[2]);

    if (f1 == -1 || f2 == -1 || f3 == -1)
    {
        completeMeshlet(meshlets, current);
        f1 = findIndex(current, tri.indices[0]);
        f2 = findIndex(current, tri.indices[1]);
        f3 = findIndex(current, tri.indices[2]);
    }
    current.primitiveLayout[current.numPrimitives * 3 + 0] = uint8(f1);
    current.primitiveLayout[current.numPrimitives * 3 + 1] = uint8(f2);
    current.primitiveLayout[current.numPrimitives * 3 + 2] = uint8(f3);
    current.numPrimitives++;
    if (current.numPrimitives == Gfx::numPrimitivesPerMeshlet)
    {
        completeMeshlet(meshlets, current);
    }
}

void Meshlet::build(const Array<Vector>& positions, const Array<uint32>& indices, Array<Meshlet>& meshlets)
{
    Meshlet current = {
        .numVertices = 0,
        .numPrimitives = 0,
    };
    Array<Triangle> triangles(indices.size() / 3);
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        triangles.add(Triangle{
            .indices = {
                indices[i * 3 + 0],
                indices[i * 3 + 1],
                indices[i * 3 + 2],
            },
            });

    }
    if (current.numVertices > 0)
    {
        completeMeshlet(meshlets, current);
    }
}
