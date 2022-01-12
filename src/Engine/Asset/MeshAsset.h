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
    const std::vector<PMesh> getMeshes();
    //Workaround while no editor
    std::vector<PMaterialAsset> referencedMaterials;
private:
    std::vector<PMesh> meshes;
};
DEFINE_REF(MeshAsset)
} // namespace Seele