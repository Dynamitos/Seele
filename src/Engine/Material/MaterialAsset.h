#pragma once
#include "Asset/Asset.h"
#include "ShaderExpression.h"

namespace Seele
{
class MaterialAsset : public Asset
{
public:
    MaterialAsset();
    MaterialAsset(const std::string &directory, const std::string &name);
    MaterialAsset(const std::string &fullPath);
    ~MaterialAsset();
protected:
    //For now its simply the collection of parameters, since there is no point for expressions
    Array<PShaderParameter> parameters;
};
DEFINE_REF(MaterialAsset);
} // namespace Seele
