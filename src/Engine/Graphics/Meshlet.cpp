#include "Meshlet.h"
#include "Containers/Map.h"
#include "Containers/List.h"
#include "Containers/Set.h"

using namespace Seele;

// Tipsy algorithm by Sanders 2007
struct Triangle
{
    bool emitted = false;
    StaticArray<uint32, 3> indices;
};

Map<uint32, List<Triangle>> buildAdjacency(const Array<uint32>& indices)
{
    Map<uint32, List<Triangle>> result;
    for (uint32 i = 0; i < indices.size(); i += 3)
    {
        result[indices[i + 0]].add(Triangle{
            .emitted = false,
            .indices = {
                indices[i + 0],
                indices[i + 1],
                indices[i + 2],
            }
            });
        result[indices[i + 1]].add(Triangle{
            .emitted = false,
            .indices = {
                indices[i + 0],
                indices[i + 1],
                indices[i + 2],
            }
            });
        result[indices[i + 2]].add(Triangle{
            .emitted = false,
            .indices = {
                indices[i + 0],
                indices[i + 1],
                indices[i + 2],
            }
            });
    }
    return result;
}

Map<uint32, uint32> getTriangleCounts(Map<uint32, List<Triangle>>& adjacency)
{
    Map<uint32, uint32> result;
    for (const auto& [index, list] : adjacency)
    {
        result[index] = list.size();
    }
    return result;
}

int32 skipDeadEnd(Map<uint32, uint32>& L, Array<uint32>& D, const Array<uint32>& indices, uint32& i, uint32 vertexCount)
{
    while (!D.empty())
    {
        uint32 d = D.back();
        D.pop();
        if (L[d] > 0)
        {
            return d;
        }
    }
    while (i < vertexCount)
    {
        i++;
        if (L[i] > 0)
        {
            return i;
        }
    }
    return -1;
}

uint32 getNextVertex(const Array<uint32>& indices, uint32& i, uint32 cacheSize, Set<uint32> N, const Array<uint32>& C, uint32 s, Map<uint32, uint32>& L, Array<uint32>& D, uint32 vertexCount)
{
    int32 n = -1;
    int32 p = -1;
    int32 m = 0;
    for (uint32 v : N)
    {
        if (L[v] > 0)
        {
            p = 0;
            if (s - C[v] + 2 * L[v] <= cacheSize)
            {
                p = s - C[v];
            }
            if (p > m)
            {
                m = p;
                n = v;
            }
        }
    }
    if (n == -1)
    {
        n = skipDeadEnd(L, D, indices, i, vertexCount);
    }
    return n;
}

Array<uint32> tipsify(const Array<Vector>& positions, const Array<uint32>& indices, uint32 cacheSize)
{
    auto A = buildAdjacency(indices);
    auto L = getTriangleCounts(A);
    auto C = Array<uint32>(positions.size(), 0);
    auto D = Array<uint32>();
    Array<uint32> output;
    int32 f = 0;
    uint32 s = cacheSize + 1, i = 1;
    while (f >= 0)
    {
        auto N = Set<uint32>();
        for (Triangle t : A[f])
        {
            if (!t.emitted)
            {
                for (uint32 v : t.indices)
                {
                    output.add(v);
                    D.add(v);
                    N.insert(v);
                    L[v] = L[v] - 1;
                    if (s - C[v] > cacheSize)
                    {
                        C[v] = s;
                        s = s + 1;
                    }
                }
                t.emitted = true;
            }
        }
        f = getNextVertex(indices, i, cacheSize, std::move(N), C, s, L, D, positions.size());
    }
    return output;
}


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

        if (f1 == -1 || f2 == -1 || f1 == -1)
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
