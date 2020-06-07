#include "MeshAsset.h"
#include "Graphics/Mesh.h"

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
    //TODO:
}
void MeshAsset::load() 
{
    //TODO:
}