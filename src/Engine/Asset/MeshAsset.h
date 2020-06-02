#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_REF(Mesh);
class MeshAsset : public Asset
{
public:
    MeshAsset();
    MeshAsset(const std::string& directory, const std::string& name);
    MeshAsset(const std::string& fullPath);
    void setMesh(PMesh mesh)
    {
        std::scoped_lock lck(lock);
        this->mesh = mesh;
    }
private:
    PMesh mesh;
};
DEFINE_REF(MeshAsset);
} // namespace Seele