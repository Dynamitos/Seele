#include "MeshAsset.h"
#include "Graphics/Mesh.h"
#include "Graphics/VertexShaderInput.h"

using namespace Seele;

MeshAsset::MeshAsset()
{
}

MeshAsset::MeshAsset(const std::string& directory, const std::string& name)
    : Asset(directory, name)
{
}
MeshAsset::MeshAsset(const std::filesystem::path& fullPath)
    : Asset(fullPath)
{
}
MeshAsset::~MeshAsset() 
{
    
}
void MeshAsset::save()
{
}
void MeshAsset::load() 
{
}

void MeshAsset::addMesh(PMesh mesh) 
{
    std::scoped_lock lck(lock);
    meshes.push_back(mesh);
    referencedMaterials.push_back(mesh->referencedMaterial);   
}

const std::vector<PMesh> MeshAsset::getMeshes()
{
    std::scoped_lock lck(lock);
    return meshes;
}