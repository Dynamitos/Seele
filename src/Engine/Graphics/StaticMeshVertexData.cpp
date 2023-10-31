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
    descriptorLayout = graphics->createDescriptorLayout("StaticMeshDescriptorLayout");
    descriptorLayout->addDescriptorBinding(0, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(1, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(2, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(3, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->addDescriptorBinding(4, Gfx::SE_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptorLayout->create();
    descriptorSet = descriptorLayout->allocateDescriptorSet();
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
        .resourceData = {
            .size = verticesAllocated * sizeof(Vector),
        },
        .stride = sizeof(Vector),
        .bDynamic = true,
    };
    positions = graphics->createShaderBuffer(createInfo);
    normals = graphics->createShaderBuffer(createInfo);
    tangents = graphics->createShaderBuffer(createInfo);
    biTangents = graphics->createShaderBuffer(createInfo);
    createInfo.resourceData.size = verticesAllocated * sizeof(Vector2);
    createInfo.stride = sizeof(Vector2);
    texCoords = graphics->createShaderBuffer(createInfo);
    
    positionData.resize(verticesAllocated);
    texCoordsData.resize(verticesAllocated);
    normalData.resize(verticesAllocated);
    tangentData.resize(verticesAllocated);
    biTangentData.resize(verticesAllocated);
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
    descriptorSet = descriptorLayout->allocateDescriptorSet();
    descriptorSet->updateBuffer(0, positions);
    descriptorSet->updateBuffer(1, texCoords);
    descriptorSet->updateBuffer(2, normals);
    descriptorSet->updateBuffer(3, tangents);
    descriptorSet->updateBuffer(4, biTangents);
    descriptorSet->writeChanges();
}
