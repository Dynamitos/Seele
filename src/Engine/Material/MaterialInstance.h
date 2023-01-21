#pragma once
#include "MaterialInterface.h"

namespace Seele
{
DECLARE_REF(Material)
class MaterialInstance : public MaterialInterface
{
public:
    MaterialInstance(PMaterial baseMaterial);
    virtual ~MaterialInstance();
    virtual Gfx::PDescriptorSet createDescriptorSet();
    virtual Gfx::PDescriptorLayout getDescriptorLayout() const;

    // The name of the generated material shader, opposed to the name of the .asset file
    virtual const std::string& getName();
    virtual const Gfx::ShaderCollection* getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const;
    virtual Gfx::ShaderCollection& createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput);

private:
    PMaterial baseMaterial;
};
DEFINE_REF(MaterialInstance)
} // namespace Seele