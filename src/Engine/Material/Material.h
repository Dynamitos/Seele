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
        Array<OShaderParameter> parameter, 
        Gfx::ODescriptorLayout layout, 
        uint32 uniformDataSize, 
        uint32 uniformBinding, 
        std::string materialName, 
        Array<OShaderExpression> expressions, 
        MaterialNode brdf);
    ~Material();
    const Gfx::PDescriptorLayout getDescriptorLayout() const { return layout; }
    PMaterialInstance instantiate();
    PMaterialInstance getInstance(uint64 instance) { return instances[instance]; }
    const std::string& getName() const { return materialName; }

    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);

    void compile();

private:
    Gfx::PGraphics graphics;
    std::string brdfName;
    uint32 uniformDataSize;
    uint32 uniformBinding;
    uint64 instanceId;
    Gfx::ODescriptorLayout layout;
    std::string materialName;
    Array<OShaderExpression> codeExpressions;
    Array<OShaderParameter> parameters;
    Array<OMaterialInstance> instances;
    MaterialNode brdf;
};
DEFINE_REF(Material)

} // namespace Seele
