#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_REF(Mesh);
DECLARE_REF(MaterialAsset);
class MeshAsset : public Asset
{
public:
    MeshAsset();
    MeshAsset(const std::string& directory, const std::string& name);
    MeshAsset(const std::filesystem::path& fullPath);
    virtual ~MeshAsset();
    virtual void save() override;
    virtual void load() override;
    void addMesh(PMesh mesh)
    {
        std::scoped_lock lck(lock);
        meshes.add(mesh);
    }
    const Array<PMesh> getMeshes() const
    {
        return meshes;
    }
private:
    Array<PMesh> meshes;
    Array<PMaterialAsset> referencedMaterials;
};
DEFINE_REF(MeshAsset);
} // namespace Seele