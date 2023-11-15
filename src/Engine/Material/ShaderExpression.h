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
    std::string source;
    ExpressionType type = ExpressionType::UNKNOWN;
    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);
};
struct ExpressionOutput
{
    std::string name;
    ExpressionType type = ExpressionType::UNKNOWN;
    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);
};
DECLARE_REF(ShaderExpression);
struct ShaderExpression
{
    Map<std::string, ExpressionInput> inputs;
    ExpressionOutput output;
    std::string key;
    ShaderExpression();
    ShaderExpression(std::string key);
    virtual ~ShaderExpression();
    virtual uint64 getIdentifier() const = 0;
    virtual std::string evaluate(Map<std::string, std::string>& varState) const = 0;
    virtual void save(ArchiveBuffer& buffer) const;
    virtual void load(ArchiveBuffer& buffer);
};
DEFINE_REF(ShaderExpression)

DECLARE_NAME_REF(Gfx, DescriptorSet)
struct ShaderParameter : public ShaderExpression
{
    uint32 byteOffset;
    uint32 binding;
    ShaderParameter() {}
    ShaderParameter(std::string name, uint32 byteOffset, uint32 binding);
    virtual ~ShaderParameter();
    // update a descriptorset, in case of a uniform buffer, copy the data to the dst + byteOffset
    virtual void updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) = 0;
    virtual void generateDeclaration(std::ofstream& stream) const = 0;
    virtual uint64 getIdentifier() const = 0;
    virtual std::string evaluate(Map<std::string, std::string>& varState) const override;
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
DECLARE_NAME_REF(Gfx, Sampler)
struct SamplerParameter : public ShaderParameter
{
    static constexpr uint64 IDENTIFIER = 0x08;
    Gfx::OSampler data;
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
    Gfx::PSampler sampler;
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
    virtual std::string evaluate(Map<std::string, std::string>& varState) const override;
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
    virtual std::string evaluate(Map<std::string, std::string>& varState) const override;
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
    virtual std::string evaluate(Map<std::string, std::string>& varState) const override;
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
    virtual std::string evaluate(Map<std::string, std::string>& varState) const override;
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
    virtual std::string evaluate(Map<std::string, std::string>& varState) const override;
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
    virtual std::string evaluate(Map<std::string, std::string>& varState) const override;
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
};
DEFINE_REF(SampleExpression)

struct MaterialNode
{
    std::string profile;
    Map<std::string, std::string> variables;
    MaterialNode() {}
    ~MaterialNode() {}
    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);
};
template<>
void Serialization::save(ArchiveBuffer& buffer, const OShaderExpression& parameter)
{
    Serialization::save(buffer, parameter->getIdentifier());
    parameter->save(buffer);
}

template<>
void Serialization::load(ArchiveBuffer& buffer, OShaderExpression& parameter)
{
    uint64 identifier = 0;
    Serialization::load(buffer, identifier);
    switch (identifier)
    {
    case FloatParameter::IDENTIFIER:
        parameter = new FloatParameter();
        break;
    case VectorParameter::IDENTIFIER:
        parameter = new VectorParameter();
        break;
    case TextureParameter::IDENTIFIER:
        parameter = new TextureParameter();
        break;
    case SamplerParameter::IDENTIFIER:
        parameter = new SamplerParameter();
        break;
    case CombinedTextureParameter::IDENTIFIER:
        parameter = new CombinedTextureParameter();
        break;
    case ConstantExpression::IDENTIFIER:
        parameter = new ConstantExpression();
        break;
    case AddExpression::IDENTIFIER:
        parameter = new AddExpression();
        break;
    case SubExpression::IDENTIFIER:
        parameter = new SubExpression();
        break;
    case MulExpression::IDENTIFIER:
        parameter = new MulExpression();
        break;
    case SwizzleExpression::IDENTIFIER:
        parameter = new SwizzleExpression();
        break;
    case SampleExpression::IDENTIFIER:
        parameter = new SampleExpression();
        break;
    default:
        throw std::runtime_error("Unknown Identifier");
    }
    parameter->load(buffer);
}

template<>
void Serialization::save(ArchiveBuffer& buffer, const OShaderParameter& parameter)
{
    Serialization::save(buffer, parameter->getIdentifier());
    parameter->save(buffer);
}

template<>
void Serialization::load(ArchiveBuffer& buffer, OShaderParameter& parameter)
{
    uint64 identifier = 0;
    Serialization::load(buffer, identifier);
    switch (identifier)
    {
    case FloatParameter::IDENTIFIER:
        parameter = new FloatParameter();
        break;
    case VectorParameter::IDENTIFIER:
        parameter = new VectorParameter();
        break;
    case TextureParameter::IDENTIFIER:
        parameter = new TextureParameter();
        break;
    case SamplerParameter::IDENTIFIER:
        parameter = new SamplerParameter();
        break;
    case CombinedTextureParameter::IDENTIFIER:
        parameter = new CombinedTextureParameter();
        break;
    default:
        throw std::runtime_error("Unknown Identifier");
    }
    parameter->load(buffer);
}

DEFINE_REF(MaterialNode)
} // namespace Seele
