#pragma once
#include "Material.h"
#include "Graphics/Buffer.h"
#include "Asset/MaterialAsset.h"

namespace Seele
{
class MaterialInstance
{
public:
    MaterialInstance();
    MaterialInstance(uint64 id, 
        Gfx::PGraphics graphics, 
        Array<OShaderExpression>& expressions,
        Array<std::string> params, 
        uint32 uniformBinding, 
        uint32 uniformSize);
    ~MaterialInstance();
    void updateDescriptor();
    Gfx::PDescriptorSet getDescriptorSet() const;
    PMaterial getBaseMaterial() const { return baseMaterial->getMaterial(); }
    uint64 getId() const { return id; }

    void setBaseMaterial(PMaterialAsset asset);

    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);

private:
    Gfx::PGraphics graphics;
    Array<uint8> uniformData;
    uint32 uniformBinding;
    Gfx::OUniformBuffer uniformBuffer;
    Array<OShaderParameter> parameters;
    Gfx::PDescriptorSet descriptor;
    PMaterialAsset baseMaterial;
    uint64 id;
};
DEFINE_REF(MaterialInstance)
} // namespace Seele