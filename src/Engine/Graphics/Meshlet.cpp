#include "Meshlet.h"
#include "Containers/Map.h"
#include "Containers/List.h"
#include "Containers/Set.h"

using namespace Seele;

struct Triangle
{
    StaticArray<uint32, 3> indices;
    Array<uint32> twoAdjacent;
    Array<uint32> oneAdjacent;
    int32 meshletId = -1;
};

int findIndex(Meshlet &current, uint32 index)
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

void completeMeshlet(Array<Meshlet> &meshlets, Meshlet &current)
{
    meshlets.add(current);
    current = {
        .boundingBox = AABB(),
        .numVertices = 0,
        .numPrimitives = 0,
    };
}

bool addTriangle(const Array<Vector>& positions, Array<Meshlet> &meshlets, Meshlet &current, Triangle tri)
{
    int f1 = findIndex(current, tri.indices[0]);
    int f2 = findIndex(current, tri.indices[1]);
    int f3 = findIndex(current, tri.indices[2]);

    if (f1 == -1 || f2 == -1 || f3 == -1)
    {
        return false;
    }
    current.boundingBox.adjust(positions[tri.indices[0]]);
    current.boundingBox.adjust(positions[tri.indices[1]]);
    current.boundingBox.adjust(positions[tri.indices[2]]);
    current.primitiveLayout[current.numPrimitives * 3 + 0] = uint8(f1);
    current.primitiveLayout[current.numPrimitives * 3 + 1] = uint8(f2);
    current.primitiveLayout[current.numPrimitives * 3 + 2] = uint8(f3);
    current.numPrimitives++;
    return true;
}

void Meshlet::build(const Array<Vector> &positions, const Array<uint32> &indices, Array<Meshlet> &meshlets)
{
    Meshlet current = {
        .numVertices = 0,
        .numPrimitives = 0,
    };
    Array<Triangle> triangles(indices.size() / 3);
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        triangles[i] = Triangle{
            .indices = {
                indices[i * 3 + 0],
                indices[i * 3 + 1],
                indices[i * 3 + 2],
            },
        };
    }
    for (size_t i = 0; i < triangles.size(); i++)
    {
        for (size_t j = 0; j < triangles.size(); j++)
        {
            if (i == j)
                continue;

            uint32 adjacency = 0;
            if (triangles[i].indices[0] == triangles[j].indices[0] 
             || triangles[i].indices[0] == triangles[j].indices[1] 
             || triangles[i].indices[0] == triangles[j].indices[2])
            {
                adjacency++;
            }
            if (triangles[i].indices[1] == triangles[j].indices[0] 
             || triangles[i].indices[1] == triangles[j].indices[1] 
             || triangles[i].indices[1] == triangles[j].indices[2])
            {
                adjacency++;
            }
            if (triangles[i].indices[1] == triangles[j].indices[0] 
             || triangles[i].indices[1] == triangles[j].indices[1] 
             || triangles[i].indices[1] == triangles[j].indices[2])
            {
                adjacency++;
            }
            if(adjacency == 2)
            {
                triangles[i].twoAdjacent.add(j);
                triangles[j].twoAdjacent.add(i);
            }
            if(adjacency == 1)
            {
                triangles[i].oneAdjacent.add(j);
                triangles[j].oneAdjacent.add(i);
            }
        }
    }
    addTriangle(positions, meshlets, current, triangles.back());
    triangles.pop();
    while(!triangles.empty())
    {
        AABB boundingBox;
        
    }
    if (current.numVertices > 0)
    {
        completeMeshlet(meshlets, current);
    }
}
