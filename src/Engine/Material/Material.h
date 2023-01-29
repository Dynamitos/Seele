#pragma once
#include "MaterialInterface.h"

namespace Seele
{
DECLARE_REF(MaterialInstance)
class Material : public MaterialInterface
{
public:
    Material(Gfx::PGraphics graphics, Array<PShaderParameter> parameter, Gfx::PDescriptorLayout layout, uint32 uniformDataSize, uint32 uniformBinding, std::string materialName);
    virtual ~Material();
    virtual Gfx::PDescriptorSet createDescriptorSet();
    virtual Gfx::PDescriptorLayout getDescriptorLayout() const { return layout; }
    virtual const std::string& getName() { return materialName; }

    PMaterialInstance instantiate();

    virtual const Gfx::ShaderCollection* getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const;
    virtual Gfx::ShaderCollection& createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput);
private:
    static Gfx::ShaderMap shaderMap;
    static std::mutex shaderMapLock;

    Gfx::PDescriptorLayout layout;
    std::string materialName;
    // With draw-indirect, we batch vertex data into big vertex buffers
    // Gfx::PVertexDataManager vertexData;

    friend class MaterialInstance;
};
DEFINE_REF(Material)
} // namespace Seele
