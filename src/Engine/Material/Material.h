#pragma once
#include "ShaderExpression.h"
#include "Graphics/Descriptor.h"

namespace Seele
{
DECLARE_REF(MaterialInstance)
class Material
{
public:
    Material();
    Material(Gfx::PGraphics graphics,
        Gfx::ODescriptorLayout layout, 
        uint32 uniformDataSize, 
        uint32 uniformBinding, 
        std::string materialName, 
        Map<std::string, OShaderExpression> expressions,
        Array<std::string> parameter,
        MaterialNode brdf);
    ~Material();
    const Gfx::PDescriptorLayout getDescriptorLayout() const { return layout; }
    OMaterialInstance instantiate();
    const std::string& getName() const { return materialName; }

    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);

    void compile();

private:
    Gfx::PGraphics graphics;
    uint32 uniformDataSize;
    uint32 uniformBinding;
    uint64 instanceId;
    Gfx::ODescriptorLayout layout;
    std::string materialName;
    Map<std::string, OShaderExpression> codeExpressions;
    Array<std::string> parameters;
    MaterialNode brdf;
};
DEFINE_REF(Material)

} // namespace Seele
