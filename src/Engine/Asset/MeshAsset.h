#pragma once
#include "Asset.h"
#include "Component/Collider.h"

namespace Seele
{
DECLARE_REF(Mesh)
DECLARE_REF(MaterialInterface)
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
    Array<PMaterialInterface> referencedMaterials;
    Array<PMesh> meshes;
    Component::Collider physicsMesh;
};
DEFINE_REF(MeshAsset)
} // namespace Seele