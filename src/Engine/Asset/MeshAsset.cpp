#include "MeshAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "AssetRegistry.h"

using namespace Seele;

MeshAsset::MeshAsset()
{
}

MeshAsset::MeshAsset(std::string_view folderPath, std::string_view name)
    : Asset(folderPath, name)
{
}

MeshAsset::~MeshAsset() 
{
}

void MeshAsset::save(ArchiveBuffer& buffer) const
{
    uint64 numMeshes = meshes.size();
    Serialization::save(buffer, numMeshes);
    for (auto mesh : meshes)
    {
        mesh->vertexInput->save(buffer);
        Array<uint8> rawIndices;
        mesh->indexBuffer->download(rawIndices);
        Serialization::save(buffer, rawIndices);
        Serialization::save(buffer, mesh->indexBuffer->getIndexType());
        Serialization::save(buffer, mesh->referencedMaterial->getAssetIdentifier());
    }
}

void MeshAsset::load(ArchiveBuffer& buffer) 
{
    uint64 numMeshes = 0;
    Serialization::load(buffer, numMeshes);
    meshes.resize(numMeshes);
    for (auto& mesh : meshes)
    {
        PVertexShaderInput vertexInput = new StaticMeshVertexInput("");
        vertexInput->load(buffer);
        Array<uint8> rawIndices;
        Serialization::load(buffer, rawIndices);
        Gfx::SeIndexType indexType;
        Serialization::load(buffer, indexType);
        IndexBufferCreateInfo createInfo = {
            .resourceData = {
                .size = rawIndices.size(),
                .data = rawIndices.data(),
            },
            .indexType = indexType,
        };
        Gfx::PIndexBuffer indexBuffer = buffer.getGraphics()->createIndexBuffer(createInfo);
        mesh = new Mesh(vertexInput, indexBuffer);
        std::string matname;
        Serialization::load(buffer, matname);
        mesh->referencedMaterial = AssetRegistry::findMaterial(matname);
    }
}

void MeshAsset::addMesh(PMesh mesh) 
{
    meshes.add(mesh);
}

const Array<PMesh> MeshAsset::getMeshes()
{
    return meshes;
}