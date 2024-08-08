#include "StaticMeshVertexData.h"
#include "Graphics.h"
#include "Graphics/Enums.h"
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
    std::memcpy((Vector4*)positions->map() + offset, data.data(), data.size() * sizeof(Vector4));
    //positions->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadTexCoords(uint64 offset, uint64 index, const Array<Vector2>& data) {
    assert(offset + data.size() <= head);
    std::memcpy((Vector2*)texCoords[index]->map() + offset, data.data(), data.size() * sizeof(Vector2));
    //texCoords[index]->updateContents(offset * sizeof(Vector2), data.size() * sizeof(Vector2), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadNormals(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy((Vector4*)normals->map() + offset, data.data(), data.size() * sizeof(Vector4));
    // normals->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadTangents(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy((Vector4*)tangents->map() + offset, data.data(), data.size() * sizeof(Vector4));
    //tangents->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadBiTangents(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy((Vector4*)biTangents->map() + offset, data.data(), data.size() * sizeof(Vector4));
    // biTangents->updateContents(offset * sizeof(Vector4), data.size() * sizeof(Vector4), data.data());
    dirty = true;
}

void StaticMeshVertexData::loadColors(uint64 offset, const Array<Vector4>& data) {
    assert(offset + data.size() <= head);
    std::memcpy((Vector4*)colors->map() + offset, data.data(), data.size() * sizeof(Vector4));
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
    Array<Vector4> pos(numVertices);
    Array<Vector2> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        tex[i].resize(numVertices);
        texCoords[i]->readContents(offset * sizeof(Vector2), numVertices * sizeof(Vector2), tex[i].data());
        Serialization::save(buffer, tex[i]);
    }
    Array<Vector4> nor(numVertices);
    Array<Vector4> tan(numVertices);
    Array<Vector4> bit(numVertices);
    Array<Vector4> col(numVertices);
    positions->readContents(offset * sizeof(Vector4), numVertices * sizeof(Vector4), pos.data());
    normals->readContents(offset * sizeof(Vector4), numVertices * sizeof(Vector4), nor.data());
    tangents->readContents(offset * sizeof(Vector4), numVertices * sizeof(Vector4), tan.data());
    biTangents->readContents(offset * sizeof(Vector4), numVertices * sizeof(Vector4), bit.data());
    colors->readContents(offset * sizeof(Vector4), numVertices * sizeof(Vector4), col.data());
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
    ShaderBufferCreateInfo createInfo = {
        .sourceData =
            {
                .size = verticesAllocated * sizeof(Vector4),
            },
        .numElements = verticesAllocated,
        .dynamic = true,
        .vertexBuffer = true,
        .name = "Positions",
    };
    positions = graphics->createShaderBuffer(createInfo);
    createInfo.vertexBuffer = false;
    createInfo.name = "Normals";
    normals = graphics->createShaderBuffer(createInfo);
    createInfo.name = "Tangents";
    tangents = graphics->createShaderBuffer(createInfo);
    createInfo.name = "BiTangents";
    biTangents = graphics->createShaderBuffer(createInfo);
    createInfo.name = "Colors";
    colors = graphics->createShaderBuffer(createInfo);
    createInfo.sourceData.size = verticesAllocated * sizeof(Vector2);
    createInfo.name = "TexCoords";
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texCoords[i] = graphics->createShaderBuffer(createInfo);
    }
    descriptorLayout = _graphics->createDescriptorLayout("pVertexData");
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 3, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = 4, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER});
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .binding = 5,
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
    positions->rotateBuffer(verticesAllocated * sizeof(Vector4), true);
    normals->rotateBuffer(verticesAllocated * sizeof(Vector4), true);
    tangents->rotateBuffer(verticesAllocated * sizeof(Vector4), true);
    biTangents->rotateBuffer(verticesAllocated * sizeof(Vector4), true);
    colors->rotateBuffer(verticesAllocated * sizeof(Vector4), true);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texCoords[i]->rotateBuffer(verticesAllocated * sizeof(Vector2), true);
    }
}

void StaticMeshVertexData::updateBuffers() {
    positions->unmap();
    normals->unmap();
    tangents->unmap();
    biTangents->unmap();
    colors->unmap();
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texCoords[i]->unmap();
    }
    descriptorLayout->reset();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, positions);
    descriptorSet->updateBuffer(1, normals);
    descriptorSet->updateBuffer(2, tangents);
    descriptorSet->updateBuffer(3, biTangents);
    descriptorSet->updateBuffer(4, colors);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        descriptorSet->updateBuffer(5, i, texCoords[i]);
    }
    descriptorSet->writeChanges();
}
