#include "MeshAsset.h"
#include "Graphics/Mesh.h"
#include "Graphics/VertexShaderInput.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

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
    boost::archive::text_oarchive archive(getWriteStream());
    archive << meshes;
}
void MeshAsset::load() 
{
    boost::archive::text_iarchive archive(getReadStream());
    archive >> meshes;
}

void MeshAsset::addMesh(PMesh mesh) 
{
    std::unique_lock lck(lock);
    meshes.add(mesh);
    referencedMaterials.add(mesh->referencedMaterial);   
}

const Array<PMesh> MeshAsset::getMeshes()
{
    std::unique_lock lck(lock);
    return meshes;
}