#include "StaticMeshVertexData.h"
#include "Graphics.h"
#include "Graphics/Enums.h"
#include "Graphics/VertexData.h"
#include "Mesh.h"
#include <fstream>

using namespace Seele;

extern List<VertexData*> vertexDataList;

StaticMeshVertexData::StaticMeshVertexData() { vertexDataList.add(this); }

StaticMeshVertexData::~StaticMeshVertexData() {}

StaticMeshVertexData* StaticMeshVertexData::getInstance() {
    static StaticMeshVertexData instance;
    return &instance;
}

void StaticMeshVertexData::loadPositions(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(posData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    //positions->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadTexCoords(uint64 offset, uint64 index, const Array<Vector2>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(texData[index].data() + offset, data.data(), data.size() * sizeof(Vector2));
    //texCoords[index]->updateContents(offset * sizeof(Vector2), data.size() * sizeof(Vector2), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadNormals(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(norData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    // normals->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadTangents(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(tanData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    //tangents->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadBiTangents(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(bitData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    // biTangents->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadColors(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(colData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    //colors->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer) {
    VertexData::serializeMesh(id, numVertices, buffer);
    uint64 offset;
    {
        std::unique_lock l(vertexDataLock);
        offset = meshOffsets[id];
    }
    Array<Vector2> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        tex[i].resize(numVertices);
        std::memcpy(tex[i].data(), texData[i].data() + offset, numVertices * sizeof(Vector2));
        Serialization::save(buffer, tex[i]);
    }
    Array<Vector4> nor(numVertices);
    Array<Vector4> tan(numVertices);
    Array<Vector4> bit(numVertices);
    Array<Vector4> col(numVertices);
    Array<Vector4> pos(numVertices);
    std::memcpy(pos.data(), posData.data() + offset, numVertices * sizeof(Vector4));
    std::memcpy(nor.data(), norData.data() + offset, numVertices * sizeof(Vector4));
    std::memcpy(tan.data(), tanData.data() + offset, numVertices * sizeof(Vector4));
    std::memcpy(bit.data(), bitData.data() + offset, numVertices * sizeof(Vector4));
    std::memcpy(col.data(), colData.data() + offset, numVertices * sizeof(Vector4));
    Serialization::save(buffer, pos);
    Serialization::save(buffer, nor);
    Serialization::save(buffer, tan);
    Serialization::save(buffer, bit);
    Serialization::save(buffer, col);
}

uint64 StaticMeshVertexData::deserializeMesh(MeshId id, ArchiveBuffer& buffer) {
    uint64 result = VertexData::deserializeMesh(id, buffer);
    uint64 offset;
    {
        std::unique_lock l(vertexDataLock);
        offset = meshOffsets[id];
    }
    Array<Vector2> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        Serialization::load(buffer, tex[i]);
        loadTexCoords(offset, i, tex[i]);
        result += tex[i].size() * sizeof(Vector2);
    }
    Array<Vector4> pos;
    Array<Vector4> nor;
    Array<Vector4> tan;
    Array<Vector4> bit;
    Array<Vector4> col;
    Serialization::load(buffer, pos);
    Serialization::load(buffer, nor);
    Serialization::load(buffer, tan);
    Serialization::load(buffer, bit);
    Serialization::load(buffer, col);
    loadPositions(offset, pos);
    loadNormals(offset, nor);
    loadTangents(offset, tan);
    loadBiTangents(offset, bit);
    loadColors(offset, col);
    result += pos.size() * sizeof(Vector4);
    result += nor.size() * sizeof(Vector4);
    result += tan.size() * sizeof(Vector4);
    result += bit.size() * sizeof(Vector4);
    result += col.size() * sizeof(Vector4);
    return result;
}

void StaticMeshVertexData::init(Gfx::PGraphics _graphics) {
    VertexData::init(_graphics);
    descriptorLayout = _graphics->createDescriptorLayout("pVertexData");
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 3, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 4, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    for(uint32 i = 0; i < MAX_TEXCOORDS; ++i)
    {
        descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
            .binding = 5 + i,
            .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        });
    }
    descriptorLayout->create();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
}

void StaticMeshVertexData::destroy() {
    VertexData::destroy();
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texCoords[i] = nullptr;
    }
    positions = nullptr;
    normals = nullptr;
    tangents = nullptr;
    biTangents = nullptr;
    colors = nullptr;
    descriptorSet = nullptr;
    descriptorLayout = nullptr;
}

void StaticMeshVertexData::bindBuffers(Gfx::PRenderCommand) {
    // TODO: for legacy vertex buffer binding
}

Gfx::PDescriptorLayout StaticMeshVertexData::getVertexDataLayout() { return descriptorLayout; }

Gfx::PDescriptorSet StaticMeshVertexData::getVertexDataSet() { return descriptorSet; }

void StaticMeshVertexData::resizeBuffers() {
    posData.resize(verticesAllocated);
    norData.resize(verticesAllocated);
    tanData.resize(verticesAllocated);
    bitData.resize(verticesAllocated);
    colData.resize(verticesAllocated);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texData[i].resize(verticesAllocated);
    }
}

void StaticMeshVertexData::updateBuffers() {
    ShaderBufferCreateInfo createInfo = {
        .sourceData =
            {
                .size = verticesAllocated * sizeof(Vector4),
                .data = (uint8*)posData.data(),
            },
        .numElements = verticesAllocated,
        .dynamic = true,
        .vertexBuffer = true,
        .name = "Positions",
    };
    positions = graphics->createShaderBuffer(createInfo);
    createInfo.vertexBuffer = false;
    createInfo.name = "Normals";
    createInfo.sourceData.data = (uint8*)norData.data();
    normals = graphics->createShaderBuffer(createInfo);
    createInfo.name = "Tangents";
    createInfo.sourceData.data = (uint8*)tanData.data();
    tangents = graphics->createShaderBuffer(createInfo);
    createInfo.name = "BiTangents";
    createInfo.sourceData.data = (uint8*)bitData.data();
    biTangents = graphics->createShaderBuffer(createInfo);
    createInfo.name = "Colors";
    createInfo.sourceData.data = (uint8*)colData.data();
    colors = graphics->createShaderBuffer(createInfo);
    createInfo.sourceData.size = verticesAllocated * sizeof(Vector2);
    createInfo.name = "TexCoords";
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        createInfo.sourceData.data = (uint8*)texData[i].data();
        texCoords[i] = graphics->createShaderBuffer(createInfo);
    }
    posData.clear();
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texData[i].clear();
    }
    norData.clear();
    tanData.clear();
    bitData.clear();
    colData.clear();
    descriptorLayout->reset();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, positions);
    descriptorSet->updateBuffer(1, normals);
    descriptorSet->updateBuffer(2, tangents);
    descriptorSet->updateBuffer(3, biTangents);
    descriptorSet->updateBuffer(4, colors);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        descriptorSet->updateBuffer(5 + i, texCoords[i]);
    }
    descriptorSet->writeChanges();
}
