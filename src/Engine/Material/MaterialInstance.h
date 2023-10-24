#pragma once
#include "Material.h"

namespace Seele
{
class MaterialInstance
{
public:
    MaterialInstance(uint64 id, Gfx::PGraphics graphics, PMaterial baseMaterial, Gfx::PDescriptorSet descriptor, Array<PShaderParameter> params, uint32 uniformBinding, uint32 uniformSize);
    ~MaterialInstance();
    void updateDescriptor();
    Gfx::PDescriptorSet getDescriptorSet() const;
    PMaterial getBaseMaterial() const { return baseMaterial; }
    uint64 getId() const { return id; }
private:
    Gfx::PGraphics graphics;
    Array<uint8> uniformData;
    uint32 uniformBinding;
    Gfx::PUniformBuffer uniformBuffer;
    Array<PShaderParameter> parameters;
    Gfx::PDescriptorSet descriptor;
    PMaterial baseMaterial;
    uint64 id;
};
DEFINE_REF(MaterialInstance)
} // namespace Seele