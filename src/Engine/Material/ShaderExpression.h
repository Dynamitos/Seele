#pragma once
#include "MinimalEngine.h"
#include "Containers/Map.h"
#include "Math/Math.h"
#include "Asset/TextureAsset.h"

namespace Seele
{
enum class ExpressionType
{
    UNKNOWN,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    TEXTURE,
    SAMPLER,
};
struct ExpressionInput
{
    int source;
    ExpressionType type = ExpressionType::UNKNOWN;
};
struct ExpressionOutput
{
    std::string name;
    ExpressionType type = ExpressionType::UNKNOWN;
};
DECLARE_REF(ShaderExpression);
struct ShaderExpression
{
    Map<std::string, ExpressionInput> inputs;
    ExpressionOutput output;
    int32 key;
    virtual uint64 getIdentifier() const = 0;
    virtual std::string evaluate(Map<int32, std::string>& varState) const = 0;
    virtual void save(ArchiveBuffer& buffer) const;
    virtual void load(ArchiveBuffer& buffer);
};
DEFINE_REF(ShaderExpression)

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
    virtual void generateDeclaration(std::ofstream& stream) const = 0;
    virtual uint64 getIdentifier() const = 0;
    virtual std::string evaluate(Map<int32, std::string>& varState) const override;
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
    virtual void generateDeclaration(std::ofstream& stream) const override;
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
    virtual void generateDeclaration(std::ofstream& stream) const override;
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
    virtual void generateDeclaration(std::ofstream& stream) const override;
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
    virtual void generateDeclaration(std::ofstream& stream) const override;
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
    virtual void generateDeclaration(std::ofstream& stream) const override;
    virtual uint64 getIdentifier() const override { return IDENTIFIER; }
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(CombinedTextureParameter)


struct ConstantExpression : public ShaderExpression
{
    static constexpr uint64 IDENTIFIER = 0x11;
    std::string expr;
    ConstantExpression();
    ConstantExpression(std::string expr, ExpressionType type);
    virtual ~ConstantExpression();
    virtual uint64 getIdentifier() const { return IDENTIFIER; }
    virtual std::string evaluate(Map<int32, std::string>& varState) const override;
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(ConstantExpression)
struct AddExpression : public ShaderExpression
{
    static constexpr uint64 IDENTIFIER = 0x12;
    AddExpression();
    virtual ~AddExpression();
    virtual uint64 getIdentifier() const { return IDENTIFIER; }
    virtual std::string evaluate(Map<int32, std::string>& varState) const override;
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(AddExpression)
struct SubExpression : public ShaderExpression
{
    static constexpr uint64 IDENTIFIER = 0x13;
    SubExpression() {}
    virtual ~SubExpression() {}
    virtual uint64 getIdentifier() const { return IDENTIFIER; }
    virtual std::string evaluate(Map<int32, std::string>& varState) const override;
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(SubExpression)
struct MulExpression : public ShaderExpression
{
    static constexpr uint64 IDENTIFIER = 0x14;
    MulExpression() {}
    virtual ~MulExpression() {}
    virtual uint64 getIdentifier() const { return IDENTIFIER; }
    virtual std::string evaluate(Map<int32, std::string>& varState) const override;
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(MulExpression)
struct SwizzleExpression : public ShaderExpression
{
    static constexpr uint64 IDENTIFIER = 0x15;
    StaticArray<int32, 4> comp = {-1, -1, -1, -1};
    SwizzleExpression() {}
    virtual ~SwizzleExpression() {}
    virtual uint64 getIdentifier() const { return IDENTIFIER; }
    virtual std::string evaluate(Map<int32, std::string>& varState) const override;
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(SwizzleExpression)
struct SampleExpression : public ShaderExpression
{
    static constexpr uint64 IDENTIFIER = 0x16;
    SampleExpression() {}
    virtual ~SampleExpression() {}
    virtual uint64 getIdentifier() const { return IDENTIFIER; }
    virtual std::string evaluate(Map<int32, std::string>& varState) const override;
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(SampleExpression)

struct MaterialNode
{
    std::string profile;
    Map<std::string, PShaderExpression> variables;
    MaterialNode() {}
    virtual ~MaterialNode() {}
    virtual std::string evaluate() const { return ""; }
    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);
};
DEFINE_REF(MaterialNode)
namespace Serialization
{
    void save(ArchiveBuffer& buffer, const PShaderExpression& parameter);
    void load(ArchiveBuffer& buffer, PShaderExpression& parameter);
    void save(ArchiveBuffer& buffer, const PShaderParameter& parameter);
    void load(ArchiveBuffer& buffer, PShaderParameter& parameter);
} // namespace Serialization
} // namespace Seele
