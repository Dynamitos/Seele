#include "Mesh.h"
#include "Graphics/Graphics.h"

using namespace Seele;

Mesh::Mesh()
{    
}

Mesh::~Mesh()
{
}

void Mesh::save(ArchiveBuffer& buffer)
{
    Serialization::save(buffer, vertexData->getTypeName());
    Serialization::save(buffer, vertexCount);
    Serialization::save(buffer, meshlets);
    vertexData->serializeMesh(id, vertexCount, buffer);
}

void Mesh::load(ArchiveBuffer& buffer)
{
    std::string typeName;
    Serialization::load(buffer, typeName);
    Serialization::load(buffer, vertexCount);
    vertexData = VertexData::findByTypeName(typeName);
    Serialization::load(buffer, meshlets);
    id = vertexData->allocateVertexData(vertexCount);
    vertexData->loadMesh(id, meshlets);
    vertexData->deserializeMesh(id, buffer);
}
