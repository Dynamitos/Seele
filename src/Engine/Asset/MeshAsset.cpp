#include "MeshAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "Graphics/VertexShaderInput.h"
#include "Material/MaterialInterface.h"

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
void MeshAsset::save(Gfx::PGraphics graphics)
{
}
void MeshAsset::load(Gfx::PGraphics graphics) 
{
}

void MeshAsset::addMesh(PMesh mesh) 
{
    std::scoped_lock lck(lock);
    meshes.add(mesh);
    referencedMaterials.add(mesh->referencedMaterial);   
}

const Array<PMesh> MeshAsset::getMeshes()
{
    std::scoped_lock lck(lock);
    return meshes;
}