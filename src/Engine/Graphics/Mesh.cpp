#include "Mesh.h"
#include "Graphics/Graphics.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

Mesh::Mesh()
{    
}

Mesh::~Mesh()
{
}

void Mesh::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, vertexData->getTypeName());
    Serialization::save(buffer, vertexCount);
    Serialization::save(buffer, indices);
    Serialization::save(buffer, meshlets);
    Serialization::save(buffer, referencedMaterial->getAssetIdentifier());
    vertexData->serializeMesh(id, vertexCount, buffer);
}

void Mesh::load(ArchiveBuffer& buffer)
{
    std::string typeName;
    Serialization::load(buffer, typeName);
    Serialization::load(buffer, vertexCount);
    vertexData = VertexData::findByTypeName(typeName);
    Serialization::load(buffer, indices);
    indexBuffer = buffer.getGraphics()->createIndexBuffer(IndexBufferCreateInfo{
        .sourceData = {
            .size = sizeof(uint32) * indices.size(),
            .data = (uint8*)indices.data(),
        },
        .indexType = Gfx::SeIndexType::SE_INDEX_TYPE_UINT32,
    });
    Serialization::load(buffer, meshlets);
    std::string refId;
    Serialization::load(buffer, refId);
    referencedMaterial = AssetRegistry::findMaterialInstance(refId);
    id = vertexData->allocateVertexData(vertexCount);
    vertexData->loadMesh(id, meshlets);
    vertexData->deserializeMesh(id, buffer);
}
