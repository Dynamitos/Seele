#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_REF(Mesh)
DECLARE_REF(MaterialAsset)
class MeshAsset : public Asset
{
public:
    MeshAsset();
    MeshAsset(const std::string& directory, const std::string& name);
    MeshAsset(const std::filesystem::path& fullPath);
    virtual ~MeshAsset();
    virtual void save() override;
    virtual void load() override;
    void addMesh(PMesh mesh);
    const Array<PMesh> getMeshes();
    //Workaround while no editor
    Array<PMaterialAsset> referencedMaterials;
    Array<PMesh> meshes;
};
DEFINE_REF(MeshAsset)
} // namespace Seele