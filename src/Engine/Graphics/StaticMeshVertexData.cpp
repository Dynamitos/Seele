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

void StaticMeshVertexData::loadPositions(uint64 offset, const Array<PositionType>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(posData.data() + offset, data.data(), data.size() * sizeof(PositionType));
    dirty = true;
}

void StaticMeshVertexData::loadTexCoords(uint64 offset, uint64 index, const Array<TexCoordType>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(texData[index].data() + offset, data.data(), data.size() * sizeof(TexCoordType));
    dirty = true;
}

void StaticMeshVertexData::loadNormals(uint64 offset, const Array<NormalType>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(norData.data() + offset, data.data(), data.size() * sizeof(NormalType));
    dirty = true;
}

void StaticMeshVertexData::loadTangents(uint64 offset, const Array<TangentType>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(tanData.data() + offset, data.data(), data.size() * sizeof(TangentType));
    dirty = true;
}

/*void StaticMeshVertexData::loadBitangents(uint64 offset, const Array<BiTangentType>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(bitData.data() + offset, data.data(), data.size() * sizeof(BiTangentType));
    dirty = true;
}*/

void StaticMeshVertexData::loadColors(uint64 offset, const Array<ColorType>& data) {
    assert(offset + data.size() <= head);
    std::memcpy(colData.data() + offset, data.data(), data.size() * sizeof(ColorType));
    dirty = true;
}

void StaticMeshVertexData::serializeMesh(MeshId id, ArchiveBuffer& buffer) {
    VertexData::serializeMesh(id, buffer);
    uint64 offset;
    uint64 numVertices;
    {
        std::unique_lock l(vertexDataLock);
        offset = meshOffsets[id];
        numVertices = meshVertexCounts[id];
    }
    Array<TexCoordType> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        tex[i].resize(numVertices);
        std::memcpy(tex[i].data(), texData[i].data() + offset, numVertices * sizeof(TexCoordType));
        Serialization::save(buffer, tex[i]);
    }
    Array<PositionType> pos(numVertices);
    Array<NormalType> nor(numVertices);
    Array<TangentType> tan(numVertices);
    //Array<BiTangentType> bit(numVertices);
    Array<ColorType> col(numVertices);
    std::memcpy(pos.data(), posData.data() + offset, numVertices * sizeof(PositionType));
    std::memcpy(nor.data(), norData.data() + offset, numVertices * sizeof(NormalType));
    std::memcpy(tan.data(), tanData.data() + offset, numVertices * sizeof(TangentType));
    //std::memcpy(bit.data(), bitData.data() + offset, numVertices * sizeof(BiTangentType));
    std::memcpy(col.data(), colData.data() + offset, numVertices * sizeof(ColorType));
    Serialization::save(buffer, pos);
    Serialization::save(buffer, nor);
    Serialization::save(buffer, tan);
    //Serialization::save(buffer, bit);
    Serialization::save(buffer, col);
}

uint64 StaticMeshVertexData::deserializeMesh(MeshId id, ArchiveBuffer& buffer) {
    uint64 result = VertexData::deserializeMesh(id, buffer);
    uint64 offset;
    {
        std::unique_lock l(vertexDataLock);
        offset = meshOffsets[id];
    }
    Array<TexCoordType> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        Serialization::load(buffer, tex[i]);
        loadTexCoords(offset, i, tex[i]);
        result += tex[i].size() * sizeof(TexCoordType);
    }
    Array<PositionType> pos;
    Array<NormalType> nor;
    Array<TangentType> tan;
    //Array<BiTangentType> bit;
    Array<ColorType> col;
    Serialization::load(buffer, pos);
    Serialization::load(buffer, nor);
    Serialization::load(buffer, tan);
    //Serialization::load(buffer, bit);
    Serialization::load(buffer, col);
    loadPositions(offset, pos);
    loadNormals(offset, nor);
    loadTangents(offset, tan);
    //loadBitangents(offset, bit);
    loadColors(offset, col);
    result += pos.size() * sizeof(PositionType);
    result += nor.size() * sizeof(NormalType);
    result += tan.size() * sizeof(TangentType);
    //result += bit.size() * sizeof(BiTangentType);
    result += col.size() * sizeof(ColorType);
    return result;
}

void StaticMeshVertexData::init(Gfx::PGraphics _graphics) {
    VertexData::init(_graphics);
    descriptorLayout = _graphics->createDescriptorLayout("pVertexData");
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = POSITIONS_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = NORMALS_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TANGENTS_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    /*descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = BITANGENTS_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });*/
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = COLORS_NAME,
        .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{
        .name = TEXCOORDS_NAME,
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
    //biTangents = nullptr;
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
    //bitData.resize(verticesAllocated);
    colData.resize(verticesAllocated);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texData[i].resize(verticesAllocated);
    }
}

void StaticMeshVertexData::updateBuffers() {
    positions = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(PositionType),
                .data = (uint8*)posData.data(),
            },
        .usage = Gfx::SE_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        .name = "Positions",
    });
    normals = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(NormalType),
                .data = (uint8*)norData.data(),
            },
        .name = "Normals",
    });
    tangents = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(TangentType),
                .data = (uint8*)tanData.data(),
            },
        .name = "Tangents",
    });
    /*biTangents = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(BiTangentType),
                .data = (uint8*)bitData.data(),
            },
        .name = "BiTangents",
    });*/
    colors = graphics->createShaderBuffer(ShaderBufferCreateInfo{
        .sourceData =
            {
                .size = verticesAllocated * sizeof(ColorType),
                .data = (uint8*)colData.data(),
            },
        .name = "Colors",
    });
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        texCoords[i] = graphics->createShaderBuffer(ShaderBufferCreateInfo{
            .sourceData =
                {
                    .size = verticesAllocated * sizeof(TexCoordType),
                    .data = (uint8*)texData[i].data(),
                },
            .name = "TexCoords",
        });
    }
    descriptorLayout->reset();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(POSITIONS_NAME, 0, positions);
    descriptorSet->updateBuffer(NORMALS_NAME, 0, normals);
    descriptorSet->updateBuffer(TANGENTS_NAME, 0, tangents);
    //descriptorSet->updateBuffer(BITANGENTS_NAME, 0, biTangents);
    descriptorSet->updateBuffer(COLORS_NAME, 0, colors);
    for (uint32 i = 0; i < MAX_TEXCOORDS; ++i) {
        descriptorSet->updateBuffer(TEXCOORDS_NAME, i, texCoords[i]);
    }
    descriptorSet->writeChanges();
}
