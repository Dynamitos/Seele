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
DECLARE_NAME_REF(Gfx, DescriptorSet)
struct ShaderParameter : public ShaderExpression
{
    std::string name;
    uint32 byteOffset;
    uint32 binding;
    ShaderParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~ShaderParameter();
    // update a descriptorset, in case of a uniform buffer, copy the data to the dst + byteOffset
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) = 0;
};
DEFINE_REF(ShaderParameter)
struct FloatParameter : public ShaderParameter
{
    float data;
    FloatParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~FloatParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
};
DEFINE_REF(FloatParameter)
struct VectorParameter : public ShaderParameter
{
    Math::Vector data;
    VectorParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~VectorParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
};
DEFINE_REF(VectorParameter)
DECLARE_REF(TextureAsset)
struct TextureParameter : public ShaderParameter
{
    PTextureAsset data;
    TextureParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~TextureParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
};
DEFINE_REF(TextureParameter)
DECLARE_NAME_REF(Gfx, SamplerState)
struct SamplerParameter : public ShaderParameter
{
    Gfx::PSamplerState data;
    SamplerParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~SamplerParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
};
DEFINE_REF(SamplerParameter)
struct CombinedTextureParameter : public ShaderParameter
{
    PTextureAsset data;
    Gfx::PSamplerState sampler;
    CombinedTextureParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~CombinedTextureParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
};
DEFINE_REF(CombinedTextureParameter)
} // namespace Seele
