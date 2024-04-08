#include "Meshlet.h"
#include "Containers/Map.h"
#include "Containers/List.h"
#include "Containers/Set.h"

using namespace Seele;

struct AdjacencyInfo
{
    Array<uint32> trianglesPerVertex;
    Array<uint32> indexBufferOffset;
    Array<uint32> triangleData;
};

void buildAdjacency(const uint32 numVerts, const Array<uint32>& indices, AdjacencyInfo& info)
{
    info.trianglesPerVertex.resize(numVerts, 0);
    for (int i = 0; i < indices.size(); ++i)
    {
        info.trianglesPerVertex[indices[i]]++;
    }
    uint32 triangleOffset = 0;
    info.indexBufferOffset.resize(numVerts, 0);
    for (int j = 0; j < numVerts; ++j)
    {
        info.indexBufferOffset[j] = triangleOffset;
        triangleOffset += info.trianglesPerVertex[j];
    }

    uint32 numTriangles = indices.size() / 3;
    info.triangleData.resize(triangleOffset);
    Array<uint32> offsets = info.indexBufferOffset;
    for (uint32 k = 0; k < numTriangles; ++k)
    {
        int a = indices[k * 3];
        int b = indices[k * 3 + 1];
        int c = indices[k * 3 + 2];

        info.triangleData[offsets[a]++] = k;
        info.triangleData[offsets[b]++] = k;
        info.triangleData[offsets[c]++] = k;
    }
}

uint32 skipDeadEnd(const Array<uint32>& liveTriCount, List<uint32>& deadEndStack, uint32& cursor)
{
    while (!deadEndStack.empty())
    {
        uint32 vertIdx = deadEndStack.front();
        deadEndStack.popFront();
        if (liveTriCount[vertIdx] > 0)
        {
            return vertIdx;
        }
    }
    while (cursor < liveTriCount.size())
    {
        if (liveTriCount[cursor] > 0)
        {
            return cursor;
        }
        ++cursor;
    }
    return -1;
}

uint32 getNextVertex(const int cacheSize, const Array<uint32>& oneRing, const Array<uint32>& cacheTimeStamps, const uint32 timeStamp, const Array<uint32>& liveTriCount, List<uint32>& deadEndStack, uint32& cursor)
{
    uint32 bestCandidate = std::numeric_limits<uint32>::max();
    int highestPriority = -1;
    for (const uint32& vertIdx : oneRing)
    {
        if (liveTriCount[vertIdx] > 0)
        {
            int priority = 0;
            if (timeStamp - cacheTimeStamps[vertIdx] + 2 * liveTriCount[vertIdx] <= cacheSize)
            {
                priority = timeStamp - cacheTimeStamps[vertIdx];
            }
            if (priority > highestPriority)
            {
                highestPriority = priority;
                bestCandidate = vertIdx;
            }
        }
    }
    if(bestCandidate == std::numeric_limits<uint32>::max())
    {
        bestCandidate = skipDeadEnd(liveTriCount, deadEndStack, cursor);
    }
    return bestCandidate;
}

void tipsifyIndexBuffer(const Array<uint32>& indices, const uint32 numVerts, const int cacheSize, Array<uint32>& outIndices)
{
    AdjacencyInfo adjacencyStruct;
    buildAdjacency(numVerts, indices, adjacencyStruct);

    Array<uint32> liveTriCount = adjacencyStruct.trianglesPerVertex;

    Array<uint32> cacheTimeStamps(numVerts);

    List<uint32> deadEndStack;

    Array<bool> emittedTriangles(indices.size() / 3);

    uint32 curVert = 0;
    uint32 timeStamp = cacheSize + 1;
    uint32 cursor = 1;
    while (curVert != -1)
    {
        Array<uint32> oneRing;
        const uint32* startTriPointer = &adjacencyStruct.triangleData[0] + adjacencyStruct.indexBufferOffset[curVert];
        const uint32* endTriPointer = startTriPointer + adjacencyStruct.trianglesPerVertex[curVert];

        for (const uint32* it = startTriPointer; it != endTriPointer; ++it)
        {
            uint32 triangle = *it;

            if (emittedTriangles[triangle])
                continue;

            uint32 a = indices[triangle * 3 + 0];
            uint32 b = indices[triangle * 3 + 1];
            uint32 c = indices[triangle * 3 + 2];

            outIndices.add(a);
            outIndices.add(b);
            outIndices.add(c);

            deadEndStack.add(a);
            deadEndStack.add(b);
            deadEndStack.add(c);

            oneRing.add(a);
            oneRing.add(b);
            oneRing.add(c);

            liveTriCount[a]--;
            liveTriCount[b]--;
            liveTriCount[c]--;

            if (timeStamp - cacheTimeStamps[a] > cacheSize) {
                cacheTimeStamps[a] = timeStamp;
            }
            if (timeStamp - cacheTimeStamps[b] > cacheSize) {
                cacheTimeStamps[b] = timeStamp;
            }
            if (timeStamp - cacheTimeStamps[c] > cacheSize) {
                cacheTimeStamps[c] = timeStamp;
            }
            emittedTriangles[triangle] = true;
        }
        curVert = getNextVertex(cacheSize, oneRing, cacheTimeStamps, timeStamp, liveTriCount, deadEndStack, cursor);
    }
}

struct Triangle
{
    StaticArray<uint32, 3> indices;
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

bool addTriangle(const Array<Vector>& positions, Meshlet &current, Triangle& tri)
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
    Array<uint32> optimizedIndices;
    tipsifyIndexBuffer(indices, positions.size(), 25, optimizedIndices);
    Array<Triangle> triangles(optimizedIndices.size() / 3);
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        triangles[i] = Triangle{
            .indices = {
                optimizedIndices[i * 3 + 0],
                optimizedIndices[i * 3 + 1],
                optimizedIndices[i * 3 + 2],
            },
        };
    }
    while(!triangles.empty())
    {
        if (!addTriangle(positions, current, triangles.back()))
        {
            completeMeshlet(meshlets, current);
            addTriangle(positions, current, triangles.back());
        }
        triangles.pop();
    }
    if (current.numVertices > 0)
    {
        completeMeshlet(meshlets, current);
    }
}
