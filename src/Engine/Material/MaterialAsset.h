#pragma once
#include "Asset/Asset.h"
#include "Graphics/GraphicsResources.h"
#include "ShaderExpression.h"

namespace Seele
{
DECLARE_REF(VertexShaderInput);
class MaterialAsset : public Asset
{
public:
    MaterialAsset();
    MaterialAsset(const std::string &directory, const std::string &name);
    MaterialAsset(const std::filesystem::path &fullPath);
    ~MaterialAsset();
    virtual void save() = 0;
    virtual void load() = 0;
    virtual std::string getMaterialName() const = 0;
    Gfx::SeBlendOp getBlendMode() const {return Gfx::SE_BLEND_OP_END_RANGE;}
    Gfx::MaterialShadingModel getShadingModel() const {return Gfx::MaterialShadingModel::DefaultLit;}

    const Gfx::ShaderCollection* getShaders(Gfx::RenderPassType renderPass, PVertexShaderInput vertexInput) const;
    Gfx::ShaderCollection& createShaders(Gfx::RenderPassType renderPass, PVertexShaderInput vertexInput);

protected:
    //For now its simply the collection of parameters, since there is no point for expressions
    Array<PShaderParameter> parameters;
    static Gfx::ShaderMap shaderMap;
};
DEFINE_REF(MaterialAsset);
} // namespace Seele
