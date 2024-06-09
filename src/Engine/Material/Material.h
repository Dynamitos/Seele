#pragma once
#include "Graphics/Descriptor.h"
#include "ShaderExpression.h"


namespace Seele {
DECLARE_REF(MaterialInstance)
class Material {
  public:
    Material();
    Material(Gfx::PGraphics graphics, Gfx::ODescriptorLayout layout, uint32 uniformDataSize, uint32 uniformBinding,
             std::string materialName, Array<OShaderExpression> expressions, Array<std::string> parameter, MaterialNode brdf);
    ~Material();
    const Gfx::PDescriptorLayout getDescriptorLayout() const { return layout; }
    Gfx::PDescriptorLayout getDescriptorLayout() { return layout; }
    OMaterialInstance instantiate();
    const std::string& getName() const { return materialName; }
    constexpr uint64 getId() const { return materialId; }
    static constexpr const PMaterial findMaterialById(uint64 id) { return materials[id]; }

    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);

    void compile();

  private:
    Gfx::PGraphics graphics;
    uint32 uniformDataSize;
    uint32 uniformBinding;
    uint64 instanceId;
    uint64 materialId;
    Gfx::ODescriptorLayout layout;
    std::string materialName;
    Array<OShaderExpression> codeExpressions;
    Array<std::string> parameters;
    MaterialNode brdf;
    static std::atomic_uint64_t materialIdCounter;
    static Array<PMaterial> materials;
};
DEFINE_REF(Material)

} // namespace Seele
