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
    Serialization::save(buffer, meshes);
}

void MeshAsset::load(ArchiveBuffer& buffer) 
{
    Serialization::load(buffer, meshes);
}
