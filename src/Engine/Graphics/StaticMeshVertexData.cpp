#include "StaticMeshVertexData.h"
#include "Graphics.h"
#include "Graphics/Enums.h"
#include "Mesh.h"

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
    uint64 offset;
    {
        std::unique_lock l(mutex);
        offset = meshOffsets[id];
    }
    assert(offset + data.size() <= head);
    std::memcpy(positionData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadTexCoords(MeshId id, uint64 index, const Array<Vector2>& data)
{
    uint64 offset;
    {
        std::unique_lock l(mutex);
        offset = meshOffsets[id];
    }
    assert(offset + data.size() <= head);
    std::memcpy(texCoordsData[index].data() + offset, data.data(), data.size() * sizeof(Vector2));
    dirty = true;
}

void StaticMeshVertexData::loadNormals(MeshId id, const Array<Vector>& data)
{
    uint64 offset;
    {
        std::unique_lock l(mutex);
        offset = meshOffsets[id];
    }
    assert(offset + data.size() <= head);
    std::memcpy(normalData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadTangents(MeshId id, const Array<Vector>& data)
{
    uint64 offset;
    {
        std::unique_lock l(mutex);
        offset = meshOffsets[id];
    }
    assert(offset + data.size() <= head);
    std::memcpy(tangentData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::loadBiTangents(MeshId id, const Array<Vector>& data)
{
    uint64 offset;
    {
        std::unique_lock l(mutex);
        offset = meshOffsets[id];
    }
    assert(offset + data.size() <= head);
    std::memcpy(biTangentData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void Seele::StaticMeshVertexData::loadColors(MeshId id, const Array<Vector>& data)
{
    uint64 offset;
    {
        std::unique_lock l(mutex);
        offset = meshOffsets[id];
    }
    assert(offset + data.size() <= head);
    std::memcpy(colorData.data() + offset, data.data(), data.size() * sizeof(Vector));
    dirty = true;
}

void StaticMeshVertexData::serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer)
{
    uint64 offset;
    {
        std::unique_lock l(mutex);
        offset = meshOffsets[id];
    }
    Array<Vector> pos(numVertices);
    Array<Vector2> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i)
    {
        tex[i].resize(numVertices);
        std::memcpy(tex[i].data(), texCoordsData[i].data() + offset, numVertices * sizeof(Vector2));
        Serialization::save(buffer, tex[i]);
    }
    Array<Vector> nor(numVertices);
    Array<Vector> tan(numVertices);
    Array<Vector> bit(numVertices);
    Array<Vector> col(numVertices);
    std::memcpy(pos.data(), positionData.data() + offset, numVertices * sizeof(Vector));
    std::memcpy(nor.data(), normalData.data() + offset, numVertices * sizeof(Vector));
    std::memcpy(tan.data(), tangentData.data() + offset, numVertices * sizeof(Vector));
    std::memcpy(bit.data(), biTangentData.data() + offset, numVertices * sizeof(Vector));
    std::memcpy(col.data(), colorData.data() + offset, numVertices * sizeof(Vector));
    Serialization::save(buffer, pos);
    Serialization::save(buffer, nor);
    Serialization::save(buffer, tan);
    Serialization::save(buffer, bit);
    Serialization::save(buffer, col);
}

void StaticMeshVertexData::deserializeMesh(MeshId id, ArchiveBuffer& buffer)
{
    Array<Vector> pos;
    Array<Vector2> tex[MAX_TEXCOORDS];
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i)
    {
        Serialization::load(buffer, tex[i]);
        loadTexCoords(id, i, tex[i]);
    }
    Array<Vector> nor;
    Array<Vector> tan;
    Array<Vector> bit;
    Array<Vector> col;
    Serialization::load(buffer, pos);
    Serialization::load(buffer, nor);
    Serialization::load(buffer, tan);
    Serialization::load(buffer, bit);
    Serialization::load(buffer, col);
    loadPositions(id, pos);
    loadNormals(id, nor);
    loadTangents(id, tan);
    loadBiTangents(id, bit);
    loadColors(id, col);
}

void StaticMeshVertexData::init(Gfx::PGraphics _graphics)
{
    VertexData::init(_graphics);
    descriptorLayout = _graphics->createDescriptorLayout("pVertexData");
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 0, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 1, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 2, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 3, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 4, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER });
    descriptorLayout->addDescriptorBinding(Gfx::DescriptorBinding{ .binding = 5, .descriptorType = Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = MAX_TEXCOORDS });
    descriptorLayout->create();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
}

void StaticMeshVertexData::destroy()
{
    VertexData::destroy();
    positions = nullptr;
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i)
    {
        texCoords[i] = nullptr;
    }
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

void StaticMeshVertexData::registerStaticMesh(const Array<OMesh>& meshes, const Component::Transform& transform)
{
    for (auto& mesh : meshes)
    {
        uint64 numVertices = meshVertexCounts[mesh->id];
        uint64 offset = meshOffsets[mesh->id];
        // Create new mesh where transform is "embedded"
        MeshId mapped = VertexData::allocateVertexData(numVertices);
        Array<Vector> pos(numVertices);
        Matrix4 matrix = transform.toMatrix();
        for (uint64 i = 0; i < numVertices; ++i)
        {
            pos[i] = matrix * Vector4(positionData[offset + i], 1);
        }
        loadPositions(mapped, pos);
        Array<Vector2> tex(numVertices);
        for (size_t i = 0; i < MAX_TEXCOORDS; ++i)
        {
            std::memcpy(tex.data(), texCoordsData[i].data() + offset, numVertices * sizeof(Vector2));
            loadTexCoords(mapped, i, tex);
        }
        Array<Vector> aux(numVertices);
        std::memcpy(aux.data(), normalData.data() + offset, numVertices * sizeof(Vector));
        loadNormals(mapped, aux);
        std::memcpy(aux.data(), biTangentData.data() + offset, numVertices * sizeof(Vector));
        loadBiTangents(mapped, aux);
        std::memcpy(aux.data(), colorData.data() + offset, numVertices * sizeof(Vector));
        loadColors(mapped, aux);

        // Load meshlets again
        VertexData::loadMesh(mapped, mesh->indices, mesh->meshlets);

        Array<uint32> ids;
        // Get references to loaded meshlets
        const auto& meshData = VertexData::getMeshData(mapped);

        for (size_t i = 0; i < meshData.numMeshlets; ++i)
        {
            ids.add(meshData.meshletOffset + i);
        }


        // Get Static instance array
        PMaterialInstance instance = mesh->referencedMaterial->getHandle();
        PMaterial baseMat = instance->getBaseMaterial();
        Array<StaticMatInstance> instances;
        instances.add(StaticMatInstance{
            .instance = mesh->referencedMaterial->getHandle(),
            .meshletIds = ids,
            .culledMeshletBuffer = graphics->createShaderBuffer(ShaderBufferCreateInfo{
            .sourceData = {
                .size = ids.size() * sizeof(uint32),
                .data = (uint8*)ids.data(),
            },
            .numElements = ids.size(),
            .dynamic = true,
            }),
            });
        staticData.add(StaticMatData{
            .material = baseMat,
            .staticInstance = std::move(instances),
            });
    }
}

void StaticMeshVertexData::resizeBuffers()
{
    ShaderBufferCreateInfo createInfo = {
        .sourceData = {
            .size = verticesAllocated * sizeof(Vector),
        },
        .numElements = verticesAllocated * 3,
        .dynamic = true,
        .name = "Positions",
    };
    positions = graphics->createShaderBuffer(createInfo);
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
    createInfo.numElements = verticesAllocated * 2;
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i)
    {
        texCoords[i] = graphics->createShaderBuffer(createInfo);
        texCoordsData[i].resize(verticesAllocated);
    }

    positionData.resize(verticesAllocated);
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
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i)
    {
        texCoords[i]->updateContents(DataSource{
            .size = texCoordsData[i].size() * sizeof(Vector2),
            .data = (uint8*)texCoordsData[i].data(),
            });
    }
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
    descriptorSet->updateBuffer(1, normals);
    descriptorSet->updateBuffer(2, tangents);
    descriptorSet->updateBuffer(3, biTangents);
    descriptorSet->updateBuffer(4, colors);
    for (size_t i = 0; i < MAX_TEXCOORDS; ++i) {
        descriptorSet->updateBuffer(5, i, texCoords[i]);
    }
    descriptorSet->writeChanges();
}
