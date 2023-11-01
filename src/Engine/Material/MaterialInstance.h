#pragma once
#include "Material.h"
#include "Graphics/Buffer.h"

namespace Seele
{
class MaterialInstance
{
public:
    MaterialInstance();
    MaterialInstance(uint64 id, Gfx::PGraphics graphics, PMaterial baseMaterial, Gfx::PDescriptorLayout descriptor, Array<OShaderParameter> params, uint32 uniformBinding, uint32 uniformSize);
    ~MaterialInstance();
    void updateDescriptor();
    Gfx::PDescriptorSet getDescriptorSet() const;
    PMaterial getBaseMaterial() const { return baseMaterial; }
    uint64 getId() const { return id; }

    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);
    void setBaseMaterial(PMaterial material);

private:
    Gfx::PGraphics graphics;
    Array<uint8> uniformData;
    uint32 uniformBinding;
    Gfx::OUniformBuffer uniformBuffer;
    Array<OShaderParameter> parameters;
    Gfx::PDescriptorLayout layout;
    Gfx::PDescriptorSet descriptor;
    PMaterial baseMaterial;
    uint64 id;
};
DEFINE_REF(MaterialInstance)
} // namespace Seele