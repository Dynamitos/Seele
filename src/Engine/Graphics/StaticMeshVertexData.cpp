#include "StaticMeshVertexData.h"
#include "Graphics.h"
#include "Graphics/Enums.h"

using namespace Seele;

extern List<VertexData*> vertexDataList;

StaticMeshVertexData::StaticMeshVertexData()
{
    vertexDataList.add(this);
}

StaticMeshVertexData::~StaticMeshVertexData()
{}

StaticMeshVertexData* StaticMeshVertexData::getInstance()
{
    static StaticMeshVertexData instance;
    return &instance;
}

void StaticMeshVertexData::loadPositions(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() <= head);
    std::memcpy(positionData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadTexCoords(MeshId id, const Array<Vector2>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() <= head);
    std::memcpy(texCoordsData.data() + offset, data.data(), data.size() * sizeof(Vector2));
    dirty = true;
}

void StaticMeshVertexData::loadNormals(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() <= head);
    std::memcpy(normalData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadTangents(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() <= head);
    std::memcpy(tangentData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadBiTangents(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() <= head);
    std::memcpy(biTangentData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void Seele::StaticMeshVertexData::loadColors(MeshId id, const Array<Vector>& data)
{
    uint64 offset = meshOffsets[id];
    assert(offset + data.size() <= head);
    std::memcpy(colorData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer)
{
    uint64 offset = meshOffsets[id];
    Array<Vector> pos(numVertices);
    Array<Vector2> tex(numVertices);
    Array<Vector> nor(numVertices);
    Array<Vector> tan(numVertices);
    Array<Vector> bit(numVertices);
    Array<Vector> col(numVertices);
    std::copy(positionData.begin() + offset, positionData.begin() + offset + numVertices, pos.begin());
    std::copy(texCoordsData.begin() + offset, texCoordsData.begin() + offset + numVertices, tex.begin());
    std::copy(normalData.begin() + offset, normalData.begin() + offset + numVertices, nor.begin());
    std::copy(tangentData.begin() + offset, tangentData.begin() + offset + numVertices, tan.begin());
    std::copy(biTangentData.begin() + offset, biTangentData.begin() + offset + numVertices, bit.begin());
    std::copy(colorData.begin() + offset, colorData.begin() + offset + numVertices, col.begin());
    Serialization::save(buffer, pos);
    Serialization::save(buffer, tex);
    Serialization::save(buffer, nor);
    Serialization::save(buffer, tan);
    Serialization::save(buffer, bit);
    Serialization::save(buffer, col);
}

void StaticMeshVertexData::deserializeMesh(MeshId id, ArchiveBuffer& buffer)
{
    Array<Vector> pos;
    Array<Vector2> tex;
    Array<Vector> nor;
    Array<Vector> tan;
    Array<Vector> bit;
    Array<Vector> col;
    Serialization::load(buffer, pos);
    Serialization::load(buffer, tex);
    Serialization::load(buffer, nor);
    Serialization::load(buffer, tan);
    Serialization::load(buffer, bit);
    Serialization::load(buffer, col);
    loadPositions(id, pos);
    loadTexCoords(id, tex);
    loadNormals(id, nor);
    loadTangents(id, tan);
    loadBiTangents(id, bit);
    loadColors(id, col);
}

void StaticMeshVertexData::init(Gfx::PGraphics graphics)
{
    VertexData::init(graphics);
    descriptorLayout = graphics->createDescriptorLayout("StaticMeshDescriptorLayout");
    descriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(5, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->create();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
}

void StaticMeshVertexData::destroy()
{
    VertexData::destroy();
    positions = nullptr;
    texCoords = nullptr;
    normals = nullptr;
    tangents = nullptr;
    biTangents = nullptr;
    colors = nullptr;
    descriptorLayout = nullptr;
}

void StaticMeshVertexData::bindBuffers(Gfx::PRenderCommand)
{
    // TODO: for legacy vertex buffer binding
}

Gfx::PDescriptorLayout StaticMeshVertexData::getVertexDataLayout()
{
    return descriptorLayout;
}

Gfx::PDescriptorSet StaticMeshVertexData::getVertexDataSet()
{
    return descriptorSet;
}

void StaticMeshVertexData::resizeBuffers()
{
    ShaderBufferCreateInfo createInfo = {
        .sourceData = {
            .size = verticesAllocated * sizeof(Vector),
        },
        .numElements = verticesAllocated * 3,
        .dynamic = true,
    };
    positions = graphics->createShaderBuffer(createInfo);
    normals = graphics->createShaderBuffer(createInfo);
    tangents = graphics->createShaderBuffer(createInfo);
    biTangents = graphics->createShaderBuffer(createInfo);
    colors = graphics->createShaderBuffer(createInfo);
    createInfo.sourceData.size = verticesAllocated * sizeof(Vector2);
    createInfo.numElements = verticesAllocated * 2;
    texCoords = graphics->createShaderBuffer(createInfo);
    
    positionData.resize(verticesAllocated);
    texCoordsData.resize(verticesAllocated);
    normalData.resize(verticesAllocated);
    tangentData.resize(verticesAllocated);
    biTangentData.resize(verticesAllocated);
    colorData.resize(verticesAllocated);
}

void StaticMeshVertexData::updateBuffers()
{
    positions->updateContents(DataSource{
        .size = positionData.size() * sizeof(Vector),
        .data = (uint8*)positionData.data(),
        });
    texCoords->updateContents(DataSource{
        .size = texCoordsData.size() * sizeof(Vector2),
        .data = (uint8*)texCoordsData.data(),
        });
    normals->updateContents(DataSource{
        .size = normalData.size() * sizeof(Vector),
        .data = (uint8*)normalData.data(),
        });
    tangents->updateContents(DataSource{
        .size = tangentData.size() * sizeof(Vector),
        .data = (uint8*)tangentData.data(),
        });
    biTangents->updateContents(DataSource{
        .size = biTangentData.size() * sizeof(Vector),
        .data = (uint8*)biTangentData.data(),
        });
    colors->updateContents(DataSource{
        .size = colorData.size() * sizeof(Vector),
        .data = (uint8*)colorData.data()
        });
    descriptorLayout->reset();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, positions);
    descriptorSet->updateBuffer(1, texCoords);
    descriptorSet->updateBuffer(2, normals);
    descriptorSet->updateBuffer(3, tangents);
    descriptorSet->updateBuffer(4, biTangents);
    descriptorSet->updateBuffer(5, colors);
    descriptorSet->writeChanges();
}
