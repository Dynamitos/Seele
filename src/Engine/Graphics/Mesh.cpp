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
    Serialization::save(buffer, transform);
    Serialization::save(buffer, vertexData->getTypeName());
    Serialization::save(buffer, vertexCount);
    Serialization::save(buffer, indices);
    Serialization::save(buffer, meshlets);
    Serialization::save(buffer, referencedMaterial->getFolderPath());
    Serialization::save(buffer, referencedMaterial->getName());
    vertexData->serializeMesh(id, vertexCount, buffer);
}

void Mesh::load(ArchiveBuffer& buffer)
{
    std::string typeName;
    Serialization::load(buffer, transform);
    Serialization::load(buffer, typeName);
    Serialization::load(buffer, vertexCount);
    vertexData = VertexData::findByTypeName(typeName);
    Serialization::load(buffer, indices);
    Serialization::load(buffer, meshlets);
    std::string refFolder;
    Serialization::load(buffer, refFolder);
    std::string refId;
    Serialization::load(buffer, refId);
    referencedMaterial = AssetRegistry::findMaterialInstance(refFolder, refId);
    id = vertexData->allocateVertexData(vertexCount);
    vertexData->loadMesh(id, indices, meshlets);
    vertexData->deserializeMesh(id, buffer);
}
