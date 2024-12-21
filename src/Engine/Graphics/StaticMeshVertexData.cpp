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

void StaticMeshVertexData::loadPositions(uint64 offset, const Array<Vector>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(posData.data() + offset, data.data(), data.size() * sizeof(Vector));
    // positions->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadTexCoords(uint64 offset, uint64 index, const Array<U16Vector2>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(texData[index].data() + offset, data.data(), data.size() * sizeof(U16Vector2));
    // texCoords[index]->updateContents(offset * sizeof(Vector2), data.size() * sizeof(Vector2), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadNormals(uint64 offset, const Array<uint32>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(norData.data() + offset, data.data(), data.size() * sizeof(uint32));
    // normals->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadColors(uint64 offset, const Array<U16Vector>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(colData.data() + offset, data.data(), data.size() * sizeof(U16Vector));
    // colors->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer) {
    VertexData::serializeMesh(id, numVertices, buffer);
    uint64 offset;
    {
        std::unique_lock l(vertexDataLock);
        offset = meshOffsets[id];
    }
    Array<U16Vector2> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        tex[i].resize(numVertices);
        std::memcpy(tex[i].data(), texData[i].data() + offset, numVertices * sizeof(U16Vector2));
        Serialization::save(buffer, tex[i]);
    }
    Array<Vector> pos(numVertices);
    Array<uint32> nor(numVertices);
    Array<U16Vector> col(numVertices);
    std::memcpy(pos.data(), posData.data() + offset, numVertices * sizeof(Vector));
    std::memcpy(nor.data(), norData.data() + offset, numVertices * sizeof(uint32));
    std::memcpy(col.data(), colData.data() + offset, numVertices * sizeof(U16Vector));
    Serialization::save(buffer, pos);
    Serialization::save(buffer, nor);
    Serialization::save(buffer, col);
}

uint64 StaticMeshVertexData::deserializeMesh(MeshId id, ArchiveBuffer& buffer) {
    uint64 result = VertexData::deserializeMesh(id, buffer);
    uint64 offset;
    {
        std::unique_lock l(vertexDataLock);
        offset = meshOffsets[id];
    }
    Array<U16Vector2> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        Serialization::load(buffer, tex[i]);
        loadTexCoords(offset, i, tex[i]);
        result += tex[i].size() * sizeof(U16Vector2);
    }
    Array<Vector> pos;
    Array<uint32> nor;
    Array<U16Vector> col;
    Serialization::load(buffer, pos);
    Serialization::load(buffer, nor);
    Serialization::load(buffer, col);
    loadPositions(offset, pos);
    loadNormals(offset, nor);
    loadColors(offset, col);
    result += pos.size() * sizeof(Vector);
    result += nor.size() * sizeof(uint32);
    result += col.size() * sizeof(U16Vector);
    return result;
}

void StaticMeshVertexData::init(Gfx::PGraphics _graphics) {
    VertexData::init(_graphics);
    descriptorLayout = _graphics->createDescriptorLayout("pVertexData");
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 3,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = MAX_TEXCOORDS,
    });
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
    colData.resize(verticesAllocated);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texData[i].resize(verticesAllocated);
    }
}

void StaticMeshVertexData::updateBuffers() {
    positions = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(Vector),
                .data = (uint8*)posData.data(),
            },
        .name = "Positions",
    });
    normals = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(uint32),
                .data = (uint8*)norData.data(),
            },
        .name = "Normals",
    });
    colors = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(U16Vector),
                .data = (uint8*)colData.data(),
            },
        .name = "Colors",
    });
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texCoords[i] = graphics->createShaderBuffer(ShaderBufferCreateInfo{
            .sourceData =
                {
                    .size = verticesAllocated * sizeof(U16Vector2),
                    .data = (uint8*)texData[i].data(),
                },
            .name = "TexCoords",
        });
    }
    descriptorLayout->reset();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, 0, positions);
    descriptorSet->updateBuffer(1, 0, normals);
    descriptorSet->updateBuffer(2, 0, colors);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        descriptorSet->updateBuffer(3, i, texCoords[i]);
    }
    descriptorSet->writeChanges();
}
