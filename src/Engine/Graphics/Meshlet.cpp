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
        triangles[i].indices[0] = indices[i * 3 + 0];
        triangles[i].indices[1] = indices[i * 3 + 1];
        triangles[i].indices[2] = indices[i * 3 + 2];
    }
    while (!triangles.empty())
    {
        uint32 best = 0;
        float lowestSurface = std::numeric_limits<float>::max();
        AABB newAABB;
        for (uint32 i = 0; i < triangles.size(); ++i)
        {
            AABB adjusted = current.boundingBox;
            adjusted.adjust(positions[triangles[i].indices[0]]);
            adjusted.adjust(positions[triangles[i].indices[1]]);
            adjusted.adjust(positions[triangles[i].indices[2]]);
            float surface = adjusted.surfaceArea();
            if (surface < lowestSurface)
            {
                lowestSurface = surface;
                best = i;
                newAABB = adjusted;
            }
        }
        current.boundingBox = newAABB;
        addTriangle(meshlets, current, triangles[best]);
        triangles.removeAt(best);
    }
}
