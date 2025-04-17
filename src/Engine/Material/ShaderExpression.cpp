#include "ShaderExpression.h"
#include "Asset/AssetRegistry.h"
#include "Asset/TextureAsset.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Graphics.h"
#include "Graphics/Resources.h"
#include "Material/Material.h"
#include <fmt/core.h>
#include <fstream>

using namespace Seele;

void ExpressionInput::save(ArchiveBuffer& buffer) const {
    Serialization::save(buffer, source);
    Serialization::save(buffer, type);
}

void ExpressionInput::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, source);
    Serialization::load(buffer, type);
}

void ExpressionOutput::save(ArchiveBuffer& buffer) const {
    Serialization::save(buffer, name);
    Serialization::save(buffer, type);
}

void ExpressionOutput::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, name);
    Serialization::load(buffer, type);
}

ShaderExpression::ShaderExpression() {}

ShaderExpression::ShaderExpression(std::string key) : key(key) {}

ShaderExpression::~ShaderExpression() {}

void ShaderExpression::save(ArchiveBuffer& buffer) const {
    Serialization::save(buffer, inputs);
    Serialization::save(buffer, output);
    Serialization::save(buffer, key);
}

void ShaderExpression::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, inputs);
    Serialization::load(buffer, output);
    Serialization::load(buffer, key);
}

ShaderParameter::ShaderParameter(std::string name, uint32 index) : ShaderExpression(name), index(index) {}

ShaderParameter::~ShaderParameter() {}

void ShaderParameter::save(ArchiveBuffer& buffer) const {
    ShaderExpression::save(buffer);
    Serialization::save(buffer, index);
}

void ShaderParameter::load(ArchiveBuffer& buffer) {
    ShaderExpression::load(buffer);
    Serialization::load(buffer, index);
}

FloatParameter::FloatParameter(std::string name, float data, uint32 index) : ShaderParameter(name, index), data(data) {
    output.name = name;
    output.type = ExpressionType::FLOAT;
}

FloatParameter::~FloatParameter() {}

void FloatParameter::updateDescriptorSet(uint32, uint32, uint32 floatOffset) { Material::updateFloatData(floatOffset + index, 1, &data); }

std::string FloatParameter::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    varState[key] = varName;
    return fmt::format("let {} = getMaterialFloatParameter({});\n", varName, index);
}

void FloatParameter::save(ArchiveBuffer& buffer) const {
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data);
}

void FloatParameter::load(ArchiveBuffer& buffer) {
    ShaderParameter::load(buffer);
    Serialization::load(buffer, data);
}

VectorParameter::VectorParameter(std::string name, Vector data, uint32 index) : ShaderParameter(name, index), data(data) {
    output.name = name;
    output.type = ExpressionType::FLOAT3;
}

VectorParameter::~VectorParameter() {}

void VectorParameter::updateDescriptorSet(uint32, uint32, uint32 floatOffset) {
    Material::updateFloatData(floatOffset + index, 3, (float*)&data);
}

std::string VectorParameter::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    varState[key] = varName;
    return fmt::format("let {} = getMaterialVectorParameter({});\n", varName, index);
}

void VectorParameter::save(ArchiveBuffer& buffer) const {
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data);
}

void VectorParameter::load(ArchiveBuffer& buffer) {
    ShaderParameter::load(buffer);
    Serialization::load(buffer, data);
}

TextureParameter::TextureParameter(std::string name, PTextureAsset asset, uint32 index) : ShaderParameter(name, index), data(asset) {
    output.name = name;
    output.type = ExpressionType::TEXTURE;
}

TextureParameter::~TextureParameter() {}

void TextureParameter::updateDescriptorSet(uint32 textureOffset, uint32, uint32) {
    Material::updateTexture(textureOffset + index, data->getTexture());
}

std::string TextureParameter::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    varState[key] = varName;
    return fmt::format("let {} = getMaterialTextureParameter({});\n", varName, index);
}

void TextureParameter::save(ArchiveBuffer& buffer) const {
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data->getFolderPath());
    Serialization::save(buffer, data->getName());
}

void TextureParameter::load(ArchiveBuffer& buffer) {
    ShaderParameter::load(buffer);
    std::string folder;
    Serialization::load(buffer, folder);
    std::string filename;
    Serialization::load(buffer, filename);
    data = AssetRegistry::findTexture(folder, filename);
}

SamplerParameter::SamplerParameter(std::string name, Gfx::OSampler sampler, uint32 index)
    : ShaderParameter(name, index), data(std::move(sampler)) {
    output.name = name;
    output.type = ExpressionType::SAMPLER;
}

SamplerParameter::~SamplerParameter() {}

void SamplerParameter::updateDescriptorSet(uint32, uint32 samplerOffset, uint32) { Material::updateSampler(samplerOffset + index, data); }

std::string SamplerParameter::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    varState[key] = varName;
    return fmt::format("let {} = getMaterialSamplerParameter({});\n", varName, index);
}

void SamplerParameter::save(ArchiveBuffer& buffer) const {
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data->samplerInfo.flags);
    Serialization::save(buffer, data->samplerInfo.magFilter);
    Serialization::save(buffer, data->samplerInfo.minFilter);
    Serialization::save(buffer, data->samplerInfo.mipmapMode);
    Serialization::save(buffer, data->samplerInfo.addressModeU);
    Serialization::save(buffer, data->samplerInfo.addressModeV);
    Serialization::save(buffer, data->samplerInfo.addressModeW);
    Serialization::save(buffer, data->samplerInfo.mipLodBias);
    Serialization::save(buffer, data->samplerInfo.anisotropyEnable);
    Serialization::save(buffer, data->samplerInfo.maxAnisotropy);
    Serialization::save(buffer, data->samplerInfo.compareEnable);
    Serialization::save(buffer, data->samplerInfo.compareOp);
    Serialization::save(buffer, data->samplerInfo.minLod);
    Serialization::save(buffer, data->samplerInfo.maxLod);
    Serialization::save(buffer, data->samplerInfo.borderColor);
    Serialization::save(buffer, data->samplerInfo.unnormalizedCoordinates);
    Serialization::save(buffer, data->samplerInfo.name);
}

void SamplerParameter::load(ArchiveBuffer& buffer) {
    ShaderParameter::load(buffer);
    SamplerCreateInfo samplerInfo;
    Serialization::load(buffer, samplerInfo.flags);
    Serialization::load(buffer, samplerInfo.magFilter);
    Serialization::load(buffer, samplerInfo.minFilter);
    Serialization::load(buffer, samplerInfo.mipmapMode);
    Serialization::load(buffer, samplerInfo.addressModeU);
    Serialization::load(buffer, samplerInfo.addressModeV);
    Serialization::load(buffer, samplerInfo.addressModeW);
    Serialization::load(buffer, samplerInfo.mipLodBias);
    Serialization::load(buffer, samplerInfo.anisotropyEnable);
    Serialization::load(buffer, samplerInfo.maxAnisotropy);
    Serialization::load(buffer, samplerInfo.compareEnable);
    Serialization::load(buffer, samplerInfo.compareOp);
    Serialization::load(buffer, samplerInfo.minLod);
    Serialization::load(buffer, samplerInfo.maxLod);
    Serialization::load(buffer, samplerInfo.borderColor);
    Serialization::load(buffer, samplerInfo.unnormalizedCoordinates);
    Serialization::load(buffer, samplerInfo.name);
    data = buffer.getGraphics()->createSampler(samplerInfo);
}

CombinedTextureParameter::CombinedTextureParameter(std::string name, PTextureAsset data, Gfx::OSampler sampler, uint32 index)
    : ShaderParameter(name, index), data(data), sampler(std::move(sampler)) {
    output.name = name;
    output.type = ExpressionType::TEXTURE;
}

CombinedTextureParameter::~CombinedTextureParameter() {}

void CombinedTextureParameter::updateDescriptorSet(uint32, uint32, uint32) { assert(false); }

std::string CombinedTextureParameter::evaluate(Map<std::string, std::string>&) const { return ""; }

void CombinedTextureParameter::save(ArchiveBuffer& buffer) const {
    ShaderParameter::save(buffer);
    Serialization::save(buffer, data->getFolderPath());
    Serialization::save(buffer, data->getName());
}

void CombinedTextureParameter::load(ArchiveBuffer& buffer) {
    ShaderParameter::load(buffer);
    std::string folder;
    Serialization::load(buffer, folder);
    std::string filename;
    Serialization::load(buffer, filename);
    data = AssetRegistry::findTexture(folder, filename);
    sampler = buffer.getGraphics()->createSampler({});
}

ConstantExpression::ConstantExpression() {}

ConstantExpression::ConstantExpression(std::string expr, ExpressionType type) : expr(expr) {
    output.name = "cexpr";
    output.type = type;
}

ConstantExpression::~ConstantExpression() {}

std::string ConstantExpression::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("const_exp_{}", key);
    varState[key] = varName;
    return fmt::format("let {} = {};\n", varName, expr);
}

void ConstantExpression::save(ArchiveBuffer& buffer) const {
    ShaderExpression::save(buffer);
    Serialization::save(buffer, expr);
}

void ConstantExpression::load(ArchiveBuffer& buffer) {
    ShaderExpression::load(buffer);
    Serialization::load(buffer, expr);
}

AddExpression::AddExpression() {}

AddExpression::~AddExpression() {}

std::string AddExpression::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    varState[key] = varName;
    std::string lhs = inputs.at("lhs").source;
    if (varState.contains(lhs)) {
        lhs = varState[lhs];
    }
    std::string rhs = inputs.at("rhs").source;
    if (varState.contains(rhs)) {
        rhs = varState[rhs];
    }

    return fmt::format("let {} = {} + {};\n", varName, lhs, rhs);
}

void AddExpression::save(ArchiveBuffer& buffer) const { ShaderExpression::save(buffer); }

void AddExpression::load(ArchiveBuffer& buffer) { ShaderExpression::load(buffer); }

std::string SubExpression::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    varState[key] = varName;
    std::string lhs = inputs.at("lhs").source;
    if (varState.contains(lhs)) {
        lhs = varState[lhs];
    }
    std::string rhs = inputs.at("rhs").source;
    if (varState.contains(rhs)) {
        rhs = varState[rhs];
    }

    return fmt::format("let {} = {} - {};\n", varName, lhs, rhs);
}

void SubExpression::save(ArchiveBuffer& buffer) const { ShaderExpression::save(buffer); }

void SubExpression::load(ArchiveBuffer& buffer) { ShaderExpression::load(buffer); }

std::string MulExpression::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    varState[key] = varName;
    std::string lhs = inputs.at("lhs").source;
    if (varState.contains(lhs)) {
        lhs = varState[lhs];
    }
    std::string rhs = inputs.at("rhs").source;
    if (varState.contains(rhs)) {
        rhs = varState[rhs];
    }
    return fmt::format("let {} = {} * {};\n", varName, lhs, rhs);
}

void MulExpression::save(ArchiveBuffer& buffer) const { ShaderExpression::save(buffer); }

void MulExpression::load(ArchiveBuffer& buffer) { ShaderExpression::load(buffer); }

std::string SwizzleExpression::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    std::string swizzle = "";
    for (uint32 i = 0; i < 4; ++i) {
        if (comp[i] == -1) {
            break;
        }
        switch (comp[i]) {
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
    return fmt::format("let {} = {}.{};\n", varName, varState[inputs.at("target").source], swizzle);
}

void SwizzleExpression::save(ArchiveBuffer& buffer) const {
    ShaderExpression::save(buffer);
    Serialization::save(buffer, comp);
}

void SwizzleExpression::load(ArchiveBuffer& buffer) {
    ShaderExpression::load(buffer);
    Serialization::load(buffer, comp);
}

std::string SampleExpression::evaluate(Map<std::string, std::string>& varState) const {
    std::string varName = fmt::format("exp_{}", key);
    varState[key] = varName;
    std::string texCoords = inputs.at("coords").source;
    if (varState.contains(inputs.at("coords").source)) {
        texCoords = varState[inputs.at("coords").source];
    }
    if (inputs.contains("texture")) {
        return fmt::format("let {} = {}.Sample({}, {});\n", varName, varState[inputs.at("texture").source],
                           varState[inputs.at("sampler").source], texCoords);
    } else {
        return fmt::format("let {} = {}.Sample({});\n", varName, varState[inputs.at("sampler").source], texCoords);
    }
}

void SampleExpression::save(ArchiveBuffer& buffer) const { ShaderExpression::save(buffer); }

void SampleExpression::load(ArchiveBuffer& buffer) { ShaderExpression::load(buffer); }

void MaterialNode::save(ArchiveBuffer& buffer) const {
    Serialization::save(buffer, profile);
    Serialization::save(buffer, variables);
}
void MaterialNode::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, profile);
    Serialization::load(buffer, variables);
}