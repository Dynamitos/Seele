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
MeshAsset::MeshAsset(const std::string& fullPath)
    : Asset(fullPath)
{
}