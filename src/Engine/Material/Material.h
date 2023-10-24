#pragma once
#include "ShaderExpression.h"
#include "Graphics/GraphicsResources.h"

namespace Seele
{
DECLARE_REF(MaterialInstance)
class Material
{
public:
    Material(Gfx::PGraphics graphics, 
        Array<PShaderParameter> parameter, 
        Gfx::PDescriptorLayout layout, 
        uint32 uniformDataSize, 
        uint32 uniformBinding, 
        std::string materialName, 
        Array<PShaderExpression> expressions, 
        MaterialNode brdf);
    ~Material();
    Gfx::PDescriptorLayout getDescriptorLayout() const { return layout; }
    PMaterialInstance instantiate();
    const std::string& getName() { return materialName; }

    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);

    void compile();

private:
    Gfx::PGraphics graphics;
    std::string brdfName;
    uint32 uniformDataSize;
    uint32 uniformBinding;
    uint64 instanceId;
    Gfx::PDescriptorLayout layout;
    std::string materialName;
    Array<PShaderExpression> codeExpressions;
    Array<PShaderParameter> parameters;
    MaterialNode brdf;
};
DEFINE_REF(Material)

} // namespace Seele
