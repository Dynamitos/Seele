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
    if (swappedOut) {
        swapIn();
    }
    assert(offset + data.size() <= head);
    std::memcpy(positionData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    dirty = true;
}

void StaticMeshVertexData::loadTexCoords(uint64 offset, uint64 index, const Array<Vector2>& data) {
    if (swappedOut) {
        swapIn();
    }
    assert(offset + data.size() <= head);
    std::memcpy(texCoordsData[index].data() + offset, data.data(), data.size() * sizeof(Vector2));
    dirty = true;
}

void StaticMeshVertexData::loadNormals(uint64 offset, const Array<Vector4>& data) {
    if (swappedOut) {
        swapIn();
    }
    assert(offset + data.size() <= head);
    std::memcpy(normalData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    dirty = true;
}

void StaticMeshVertexData::loadTangents(uint64 offset, const Array<Vector4>& data) {
    if (swappedOut) {
        swapIn();
    }
    assert(offset + data.size() <= head);
    std::memcpy(tangentData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    dirty = true;
}

void StaticMeshVertexData::loadBiTangents(uint64 offset, const Array<Vector4>& data) {
    if (swappedOut) {
        swapIn();
    }
    assert(offset + data.size() <= head);
    std::memcpy(biTangentData.data() + offset, data.data(), data.size() * sizeof(Vector4));
    dirty = true;
}

void Seele::StaticMeshVertexData::loadColors(uint64 offset, const Array<Vector4>& data) {
    if (swappedOut) {
        swapIn();
    }
    assert(offset + data.size() <= head);
    std::memcpy(colorData.data() + offset, data.data(), data.size() * sizeof(Vector4));
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
        std::memcpy(tex[i].data(), texCoordsData[i].data() + offset, numVertices * sizeof(Vector2));
        Serialization::save(buffer, tex[i]);
    }
    Array<Vector4> nor(numVertices);
    Array<Vector4> tan(numVertices);
    Array<Vector4> bit(numVertices);
    Array<Vector4> col(numVertices);
    std::memcpy(pos.data(), positionData.data() + offset, numVertices * sizeof(Vector4));
    std::memcpy(nor.data(), normalData.data() + offset, numVertices * sizeof(Vector4));
    std::memcpy(tan.data(), tangentData.data() + offset, numVertices * sizeof(Vector4));
    std::memcpy(bit.data(), biTangentData.data() + offset, numVertices * sizeof(Vector4));
    std::memcpy(col.data(), colorData.data() + offset, numVertices * sizeof(Vector4));
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
    Array<Vector4> pos;
    Array<Vector2> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        Serialization::load(buffer, tex[i]);
        loadTexCoords(offset, i, tex[i]);
        result += tex[i].size() * sizeof(Vector2);
    }
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
    positions = nullptr;
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texCoords[i] = nullptr;
    }
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
    ShaderBufferCreateInfo createInfo = {
        .sourceData =
            {
                .size = verticesAllocated * sizeof(Vector4),
            },
        .numElements = verticesAllocated,
        .dynamic = false,
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
        texCoordsData[i].resize(verticesAllocated);
    }

    positionData.resize(verticesAllocated);
    normalData.resize(verticesAllocated);
    tangentData.resize(verticesAllocated);
    biTangentData.resize(verticesAllocated);
    colorData.resize(verticesAllocated);
}

void StaticMeshVertexData::updateBuffers() {
    positions->updateContents(ShaderBufferCreateInfo{
        .sourceData{
            .size = positionData.size() * sizeof(Vector4),
            .data = (uint8*)positionData.data(),
        },
        .numElements = positionData.size(),
    });
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texCoords[i]->updateContents(ShaderBufferCreateInfo{
            .sourceData =
                {
                    .size = texCoordsData[i].size() * sizeof(Vector2),
                    .data = (uint8*)texCoordsData[i].data(),
                },
            .numElements = texCoordsData[i].size(),
        });
    }
    normals->updateContents(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = normalData.size() * sizeof(Vector4),
                .data = (uint8*)normalData.data(),
            },
        .numElements = normalData.size(),
    });
    tangents->updateContents(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = tangentData.size() * sizeof(Vector4),
                .data = (uint8*)tangentData.data(),
            },
        .numElements = tangentData.size(),
    });
    biTangents->updateContents(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = biTangentData.size() * sizeof(Vector4),
                .data = (uint8*)biTangentData.data(),
            },
        .numElements = biTangentData.size(),
    });
    colors->updateContents(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = colorData.size() * sizeof(Vector4),
                .data = (uint8*)colorData.data(),
            },
        .numElements = colorData.size(),
    });
    // we just updated the GPU buffers, might not change that for a while
    swapOut();
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

void StaticMeshVertexData::swapOut() {
    ArchiveBuffer buf;
    Serialization::save(buf, positionData);
    positionData.clear();
    Serialization::save(buf, normalData);
    normalData.clear();
    Serialization::save(buf, tangentData);
    tangentData.clear();
    Serialization::save(buf, biTangentData);
    biTangentData.clear();
    Serialization::save(buf, colorData);
    colorData.clear();
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        Serialization::save(buf, texCoordsData[i]);
        texCoordsData[i].clear();
    }
    std::ofstream str("vertex", std::ios::binary);
    buf.writeToStream(str);
    swappedOut = true;
}

void StaticMeshVertexData::swapIn() {
    ArchiveBuffer buf;
    std::ifstream str("vertex", std::ios::binary);
    buf.readFromStream(str);
    Serialization::load(buf, positionData);
    Serialization::load(buf, normalData);
    Serialization::load(buf, tangentData);
    Serialization::load(buf, biTangentData);
    Serialization::load(buf, colorData);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        Serialization::load(buf, texCoordsData[i]);
    }
    swappedOut = false;
}
