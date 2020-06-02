#pragma once
#include "MinimalEngine.h"

namespace Seele
{
struct ExpressionInput
{

};
struct ExpressionOutput
{

};
struct ShaderExpression
{
};
struct ShaderParameter : public ShaderExpression
{
    std::string name;
};
DEFINE_REF(ShaderParameter);
struct FloatParameter : public ShaderParameter
{
    float defaultValue;
    bool setParameter(const std::string& paramName, float newDefault)
    {
        if(name.compare(paramName) == 0)
        {
            defaultValue = newDefault;
            return true;
        }
        return false;
    }
};
DEFINE_REF(FloatParameter);
struct VectorParameter : public ShaderParameter
{
    Vector defaultValue;
    bool setParameter(const std::string& paramName, Vector newDefault)
    {
        if(name.compare(paramName) == 0)
        {
            defaultValue = newDefault;
            return true;
        }
        return false;
    }
};
DEFINE_REF(VectorParameter);
DECLARE_NAME_REF(Gfx, Texture2D);
struct TextureParameter : public ShaderParameter
{
    Gfx::PTexture2D defaultValue;
    bool setParameter(const std::string& paramName, Gfx::PTexture2D newDefault)
    {
        if(name.compare(paramName) == 0)
        {
            return true;
        }
        return false;
    }
};
DEFINE_REF(TextureParameter);
DECLARE_NAME_REF(Gfx, SamplerState);
struct SamplerParameter : public ShaderParameter
{
    Gfx::PSamplerState defaultValue;
    bool setParameter(const std::string& paramName, Gfx::PSamplerState newDefault)
    {
        if(name.compare(paramName) == 0)
        {
            return true;
        }
        return false;
    }
};
DEFINE_REF(SamplerParameter);

} // namespace Seele
