#pragma once
#include "MinimalEngine.h"
#include "Math/Math.h"
#include "Asset/TextureAsset.h"

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
    ShaderParameter() {}
    ShaderParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~ShaderParameter();
    // update a descriptorset, in case of a uniform buffer, copy the data to the dst + byteOffset
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) = 0;
    virtual uint64 getIdentifier() const = 0;
    virtual void save(ArchiveBuffer& buffer) const;
    virtual void load(ArchiveBuffer& buffer);
};
DEFINE_REF(ShaderParameter)
struct FloatParameter : public ShaderParameter
{
    static constexpr uint64 IDENTIFIER = 0x01;
    float data;
    FloatParameter() {}
    FloatParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~FloatParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
    virtual uint64 getIdentifier() const override { return IDENTIFIER; }
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(FloatParameter)
struct VectorParameter : public ShaderParameter
{
    static constexpr uint64 IDENTIFIER = 0x02;
    Vector data;
    VectorParameter() {}
    VectorParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~VectorParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
    virtual uint64 getIdentifier() const override { return IDENTIFIER; }
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(VectorParameter)
struct TextureParameter : public ShaderParameter
{
    static constexpr uint64 IDENTIFIER = 0x04;
    PTextureAsset data;
    TextureParameter() {}
    TextureParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~TextureParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
    virtual uint64 getIdentifier() const override { return IDENTIFIER; }
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(TextureParameter)
DECLARE_NAME_REF(Gfx, SamplerState)
struct SamplerParameter : public ShaderParameter
{
    static constexpr uint64 IDENTIFIER = 0x08;
    Gfx::PSamplerState data;
    SamplerParameter() {}
    SamplerParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~SamplerParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
    virtual uint64 getIdentifier() const override { return IDENTIFIER; }
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(SamplerParameter)
struct CombinedTextureParameter : public ShaderParameter
{
    static constexpr uint64 IDENTIFIER = 0x10;
    PTextureAsset data;
    Gfx::PSamplerState sampler;
    CombinedTextureParameter() {}
    CombinedTextureParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~CombinedTextureParameter();
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) override;
    virtual uint64 getIdentifier() const override { return IDENTIFIER; }
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(CombinedTextureParameter)
namespace Serialization
{
    void save(ArchiveBuffer& buffer, const PShaderParameter& parameter);
    void load(ArchiveBuffer& buffer, PShaderParameter& parameter);
} // namespace Serialization
} // namespace Seele
