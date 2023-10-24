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
    static constexpr uint64 IDENTIFIER = 0x2;
    MeshAsset();
    MeshAsset(std::string_view folderPath, std::string_view name);
    virtual ~MeshAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
    //Workaround while no editor
    Array<PMesh> meshes;
    Component::Collider physicsMesh;
};
DEFINE_REF(MeshAsset)
} // namespace Seele