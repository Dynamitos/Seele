#include "StaticMeshVertexData.h"
#include "Graphics.h"

using namespace Seele;

extern List<VertexData*> vertexDataList;

constexpr static uint64 NUM_DEFAULT_ELEMENTS = 1024;

StaticMeshVertexData::StaticMeshVertexData()
    : head(0)
    , verticesAllocated(NUM_DEFAULT_ELEMENTS)
{
    vertexDataList.add(this);
}

StaticMeshVertexData::~StaticMeshVertexData()
{}

StaticMeshVertexData* Seele::StaticMeshVertexData::getInstance()
{
    return ;
}

MeshId StaticMeshVertexData::allocateVertexData(uint64 numVertices)
{
    MeshId res{ idCounter++ };
    meshOffsets[res] = head;
    head += numVertices;
    if (head > verticesAllocated)
    {
        ShaderBufferCreateInfo createInfo = {
            .resourceData = {
                .size = head * sizeof(Vector),
            },
            .stride = sizeof(Vector),
            .bDynamic = true,
        };
        positions = graphics->createShaderBuffer(createInfo);
        normals = graphics->createShaderBuffer(createInfo);
        tangents = graphics->createShaderBuffer(createInfo);
        biTangents = graphics->createShaderBuffer(createInfo);
        createInfo.resourceData.size = head * sizeof(Vector2);
        createInfo.stride = sizeof(Vector2);
        texCoords = graphics->createShaderBuffer(createInfo);
    }
    positionData.resize(head);
    texCoordsData.resize(head);
    normalData.resize(head);
    tangentData.resize(head);
    biTangentData.resize(head);
    return res;
}

void StaticMeshVertexData::loadPositions(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() < head);
    std::memcpy(positionData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadTexCoords(MeshId id, const Array<Vector2>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() < head);
    std::memcpy(texCoordsData.data() + offset, data.data(), data.size() * sizeof(Vector2));
    dirty = true;
}

void StaticMeshVertexData::loadNormals(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() < head);
    std::memcpy(normalData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadTangents(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() < head);
    std::memcpy(tangentData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadBiTangents(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() < head);
    std::memcpy(biTangentData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::init(Gfx::PGraphics graphics)
{
    VertexData::init(graphics);
    ShaderBufferCreateInfo createInfo = {
        .resourceData = {
            .size = NUM_DEFAULT_ELEMENTS * sizeof(Vector),
        },
        .stride = sizeof(Vector),
        .bDynamic = true,
    };
    positions = graphics->createShaderBuffer(createInfo);
    normals = graphics->createShaderBuffer(createInfo);
    tangents = graphics->createShaderBuffer(createInfo);
    biTangents = graphics->createShaderBuffer(createInfo);
    createInfo.resourceData.size = NUM_DEFAULT_ELEMENTS * sizeof(Vector2);
    createInfo.stride = sizeof(Vector2);
    texCoords = graphics->createShaderBuffer(createInfo);
}

void StaticMeshVertexData::updateBuffers()
{
    positions->updateContents(BulkResourceData{
        .size = positionData.size() * sizeof(Vector),
        .data = (uint8*)positionData.data(),
        });
    texCoords->updateContents(BulkResourceData{
        .size = texCoordsData.size() * sizeof(Vector2),
        .data = (uint8*)texCoordsData.data(),
        });
    normals->updateContents(BulkResourceData{
        .size = normalData.size() * sizeof(Vector),
        .data = (uint8*)normalData.data(),
        });
    tangents->updateContents(BulkResourceData{
        .size = tangentData.size() * sizeof(Vector),
        .data = (uint8*)tangentData.data(),
        });
    biTangents->updateContents(BulkResourceData{
        .size = biTangentData.size() * sizeof(Vector),
        .data = (uint8*)biTangentData.data(),
        });
}
