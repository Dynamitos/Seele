#include "Mesh.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/Graphics.h"

using namespace Seele;

Mesh::Mesh() {}

Mesh::~Mesh() {}

void Mesh::save(ArchiveBuffer& buffer) const {
    Serialization::save(buffer, transform);
    Serialization::save(buffer, vertexData->getTypeName());
    Serialization::save(buffer, vertexCount);
    Serialization::save(buffer, referencedMaterial->getFolderPath());
    Serialization::save(buffer, referencedMaterial->getName());
    vertexData->serializeMesh(id, buffer);
}

void Mesh::load(ArchiveBuffer& buffer) {
    std::string typeName;
    Serialization::load(buffer, transform);
    Serialization::load(buffer, typeName);
    Serialization::load(buffer, vertexCount);
    vertexData = VertexData::findByTypeName(typeName);
    std::string refFolder;
    Serialization::load(buffer, refFolder);
    std::string refId;
    Serialization::load(buffer, refId);
    referencedMaterial = AssetRegistry::findMaterialInstance(refFolder, refId);
    id = vertexData->allocateVertexData(vertexCount);
    byteSize = vertexData->deserializeMesh(id, buffer);
    if (buffer.getGraphics()->supportRayTracing()) {
        blas = buffer.getGraphics()->createBottomLevelAccelerationStructure(Gfx::BottomLevelASCreateInfo{
            .mesh = this,
        });
        vertexData->registerBottomLevelAccelerationStructure(blas);
    }
}

uint64 Mesh::getCPUSize() const {
    uint64 result = sizeof(Mesh);
    result += byteSize;
    return result;
}

uint64 Mesh::getGPUSize() const { return byteSize; }
