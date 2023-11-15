#include "ShaderExpression.h"
#include "Graphics/Resources.h"
#include "Asset/TextureAsset.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/Graphics.h"
#include "Graphics/Descriptor.h"
#include <format>

using namespace Seele;

void ExpressionInput::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, source);
    Serialization::save(buffer, type);
}

void ExpressionInput::load(ArchiveBuffer& buffer)
{
    Serialization::load(buffer, source);
    Serialization::load(buffer, type);
}

void ExpressionOutput::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, name);
    Serialization::save(buffer, type);
}

void ExpressionOutput::load(ArchiveBuffer& buffer)
{
    Serialization::load(buffer, name);
    Serialization::load(buffer, type);
}

ShaderExpression::ShaderExpression()
{
}

ShaderExpression::ShaderExpression(std::string key)
    : key(key)
{
}

ShaderExpression::~ShaderExpression()
{
}

void ShaderExpression::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, inputs);
    Serialization::save(buffer, output);
    Serialization::save(buffer, key);
}

void ShaderExpression::load(ArchiveBuffer& buffer)
{
    Serialization::load(buffer, inputs);
    Serialization::load(buffer, output);
    Serialization::load(buffer, key);
}

ShaderParameter::ShaderParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderExpression(name)
    , byteOffset(byteOffset)
    , binding(binding)
{
}

ShaderParameter::~ShaderParameter() 
{
}

std::string ShaderParameter::evaluate(Map<std::string, std::string>& varState) const
{
    varState[key] = key;
    return "";
}

void ShaderParameter::save(ArchiveBuffer& buffer) const
{
    ShaderExpression::save(buffer);
    Serialization::save(buffer, byteOffset);
    Serialization::save(buffer, binding);
}

void ShaderParameter::load(ArchiveBuffer& buffer)
{
    ShaderExpression::load(buffer);
    Serialization::load(buffer, byteOffset);
    Serialization::load(buffer, binding);
}

FloatParameter::FloatParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
    , data(0)
{
    output.name = name;
    output.type = ExpressionType::FLOAT;
}

FloatParameter::~FloatParameter() 
{
}

void FloatParameter::save(ArchiveBuffer& buffer) const
{
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data);
}

void FloatParameter::load(ArchiveBuffer& buffer)
{
    ShaderParameter::load(buffer);
    Serialization::load(buffer, data);
}

void FloatParameter::updateDescriptorSet(Gfx::PDescriptorSet, uint8* dst) 
{
    std::memcpy(dst + byteOffset, &data, sizeof(float));
}

void FloatParameter::generateDeclaration(std::ofstream& stream) const
{
    stream << "\tlayout(offset = " << byteOffset << ") float " << key << ";\n";
}

VectorParameter::VectorParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
    , data()
{
    output.name = name;
    output.type = ExpressionType::FLOAT3;
}

VectorParameter::~VectorParameter() 
{
}

void VectorParameter::updateDescriptorSet(Gfx::PDescriptorSet, uint8* dst) 
{
    std::memcpy(dst + byteOffset, &data, sizeof(Vector));
}

void VectorParameter::generateDeclaration(std::ofstream& stream) const
{
    stream << "\tlayout(offset = " << byteOffset << ") float3 " << key << ";\n";
}

void VectorParameter::save(ArchiveBuffer& buffer) const
{
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data);
}

void VectorParameter::load(ArchiveBuffer& buffer)
{
    ShaderParameter::load(buffer);
    Serialization::load(buffer, data);
}

TextureParameter::TextureParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
{
    output.name = name;
    output.type = ExpressionType::TEXTURE;
}

TextureParameter::~TextureParameter() 
{
}

void TextureParameter::updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8*) 
{
    descriptorSet->updateTexture(binding, data->getTexture());
}

void TextureParameter::generateDeclaration(std::ofstream& stream) const
{
    stream << "\tTexture2D " << key << ";\n";
}

void TextureParameter::save(ArchiveBuffer& buffer) const
{
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data->getAssetIdentifier());
}

void TextureParameter::load(ArchiveBuffer& buffer)
{
    ShaderParameter::load(buffer);
    std::string filename;
    Serialization::load(buffer, filename);
    data = AssetRegistry::findTexture(filename);
}

SamplerParameter::SamplerParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
{
    output.name = name;
    output.type = ExpressionType::SAMPLER;
}

SamplerParameter::~SamplerParameter() 
{
    
}

void SamplerParameter::updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8*) 
{
    descriptorSet->updateSampler(binding, data);
}

void SamplerParameter::generateDeclaration(std::ofstream& stream) const
{
    stream << "\tSamplerState " << key << ";\n";
}

void SamplerParameter::save(ArchiveBuffer& buffer) const
{
    ShaderParameter::save(buffer);
}

void SamplerParameter::load(ArchiveBuffer& buffer)
{
    ShaderParameter::load(buffer);
    data = buffer.getGraphics()->createSampler({});
}

CombinedTextureParameter::CombinedTextureParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
{
    output.name = name;
    output.type = ExpressionType::TEXTURE;
}

CombinedTextureParameter::~CombinedTextureParameter() 
{
    
}

void CombinedTextureParameter::updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8*) 
{
    descriptorSet->updateTexture(binding, data->getTexture(), sampler);
}

void CombinedTextureParameter::generateDeclaration(std::ofstream& stream) const
{
    stream << "\tTexture2D " << key << ";\n";
}

void CombinedTextureParameter::save(ArchiveBuffer& buffer) const
{
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data->getAssetIdentifier());
}

void CombinedTextureParameter::load(ArchiveBuffer& buffer)
{
    ShaderParameter::load(buffer);
    std::string filename;
    Serialization::load(buffer, filename);
    data = AssetRegistry::findTexture(filename);
    sampler = buffer.getGraphics()->createSampler({});
}

ConstantExpression::ConstantExpression()
{
}

ConstantExpression::ConstantExpression(std::string expr, ExpressionType type)
    : expr(expr)
{
    output.name = "cexpr";
    output.type = type;
}

ConstantExpression::~ConstantExpression()
{
    
}

std::string ConstantExpression::evaluate(Map<std::string, std::string>& varState) const
{
    std::string varName = std::format("const_exp_{}", key);
    varState[key] = varName;
    return std::format("let {} = {};\n", varName, expr);
}

void ConstantExpression::save(ArchiveBuffer& buffer) const 
{
    ShaderExpression::save(buffer);
    Serialization::save(buffer, expr);
}

void ConstantExpression::load(ArchiveBuffer& buffer)
{
    ShaderExpression::load(buffer);
    Serialization::load(buffer, expr);
}

AddExpression::AddExpression()
{

}

AddExpression::~AddExpression()
{
    
}

std::string AddExpression::evaluate(Map<std::string, std::string>& varState) const
{
    std::string varName = std::format("exp_{}", key);
    varState[key] = varName;
    return std::format("let {} = {} + {};\n", varName, varState[inputs.at("lhs").source], varState[inputs.at("rhs").source]);
}

void AddExpression::save(ArchiveBuffer& buffer) const 
{
    ShaderExpression::save(buffer);
}

void AddExpression::load(ArchiveBuffer& buffer)
{
    ShaderExpression::load(buffer);
}

std::string SubExpression::evaluate(Map<std::string, std::string>& varState) const
{
    std::string varName = std::format("exp_{}", key);
    varState[key] = varName;
    return std::format("let {} = {} - {};\n", varName, varState[inputs.at("lhs").source], varState[inputs.at("rhs").source]);
}

void SubExpression::save(ArchiveBuffer& buffer) const 
{
    ShaderExpression::save(buffer);
}

void SubExpression::load(ArchiveBuffer& buffer)
{
    ShaderExpression::load(buffer);
}

std::string MulExpression::evaluate(Map<std::string, std::string>& varState) const 
{
    std::string varName = std::format("exp_{}", key);
    varState[key] = varName;
    return std::format("let {} = mul({}, {});\n", varName, varState[inputs.at("lhs").source], varState[inputs.at("rhs").source]);
}

void MulExpression::save(ArchiveBuffer& buffer) const 
{
    ShaderExpression::save(buffer);
}

void MulExpression::load(ArchiveBuffer& buffer)
{
    ShaderExpression::load(buffer);
}

std::string SwizzleExpression::evaluate(Map<std::string, std::string>& varState) const
{
    std::string varName = std::format("exp_{}", key);
    std::string swizzle = "";
    for(uint32 i = 0; i < 4; ++i)
    {
        if(comp[i] == -1)
        {
            break;
        }
        switch (comp[i])
        {
        case 0:
            swizzle += "x";
            break;
        case 1:
            swizzle += "y";
            break;
        case 2:
            swizzle += "z";
            break;
        case 3:
            swizzle += "w";
            break;
        default:
            throw std::logic_error("invalid component");
        }
    }
    varState[key] = varName;
    return std::format("let {} = {}.{};\n", varName, varState[inputs.at("target").source], swizzle);
}

void SwizzleExpression::save(ArchiveBuffer& buffer) const 
{
    ShaderExpression::save(buffer);
    Serialization::save(buffer, comp);
}

void SwizzleExpression::load(ArchiveBuffer& buffer)
{
    ShaderExpression::load(buffer);
    Serialization::load(buffer, comp);
}

std::string SampleExpression::evaluate(Map<std::string, std::string>& varState) const 
{
    std::string varName = std::format("exp_{}", key);
    varState[key] = varName;
    return std::format("let {} = {}.Sample({}, {});\n", varName, varState[inputs.at("texture").source], varState[inputs.at("sampler").source], varState[inputs.at("coords").source]);
}

void SampleExpression::save(ArchiveBuffer& buffer) const 
{
    ShaderExpression::save(buffer);
}

void SampleExpression::load(ArchiveBuffer& buffer)
{
    ShaderExpression::load(buffer);
}

void MaterialNode::save(ArchiveBuffer& buffer) const 
{
    Serialization::save(buffer, profile);
    Serialization::save(buffer, variables);
}
void MaterialNode::load(ArchiveBuffer& buffer) 
{
    Serialization::load(buffer, profile);
    Serialization::load(buffer, variables);
}