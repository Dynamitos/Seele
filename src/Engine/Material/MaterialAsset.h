#pragma once
#include "Asset/Asset.h"
#include "Graphics/GraphicsResources.h"
#include "ShaderExpression.h"

namespace Seele
{
DECLARE_REF(VertexShaderInput);
DECLARE_REF(Material);
class MaterialAsset : public Asset
{
public:
    MaterialAsset();
    MaterialAsset(const std::string &directory, const std::string &name);
    MaterialAsset(const std::filesystem::path &fullPath);
    ~MaterialAsset();
    virtual void save() = 0;
    virtual void load() = 0;
    virtual PMaterial getRenderMaterial() = 0;
    Gfx::SeBlendOp getBlendMode() const {return Gfx::SE_BLEND_OP_END_RANGE;}
    Gfx::MaterialShadingModel getShadingModel() const {return Gfx::MaterialShadingModel::DefaultLit;}

protected:
    //For now its simply the collection of parameters, since there is no point for expressions
    Array<PShaderParameter> parameters;
};
DEFINE_REF(MaterialAsset);
} // namespace Seele
