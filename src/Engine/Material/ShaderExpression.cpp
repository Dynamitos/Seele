#include "ShaderExpression.h"
#include "Graphics/GraphicsResources.h"
#include "Asset/TextureAsset.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/Graphics.h"

using namespace Seele;

ShaderParameter::ShaderParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : name(name)
    , byteOffset(byteOffset)
    , binding(binding)
{
}

ShaderParameter::~ShaderParameter() 
{
}

void ShaderParameter::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, name);
    Serialization::save(buffer, byteOffset);
    Serialization::save(buffer, binding);
}

void ShaderParameter::load(ArchiveBuffer& buffer)
{
    Serialization::load(buffer, name);
    Serialization::load(buffer, byteOffset);
    Serialization::load(buffer, binding);
}

FloatParameter::FloatParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
    , data(0)
{
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

VectorParameter::VectorParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
    , data()
{
}

VectorParameter::~VectorParameter() 
{
}


void VectorParameter::updateDescriptorSet(Gfx::PDescriptorSet, uint8* dst) 
{
    std::memcpy(dst + byteOffset, &data, sizeof(Vector));
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
}

TextureParameter::~TextureParameter() 
{
}

void TextureParameter::updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8*) 
{
    descriptorSet->updateTexture(binding, data->getTexture());
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
}

SamplerParameter::~SamplerParameter() 
{
    
}

void SamplerParameter::updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8*) 
{
    descriptorSet->updateSampler(binding, data);
}


void SamplerParameter::save(ArchiveBuffer& buffer) const
{
    ShaderParameter::save(buffer);
}

void SamplerParameter::load(ArchiveBuffer& buffer)
{
    ShaderParameter::load(buffer);
    data = buffer.getGraphics()->createSamplerState({});
}

CombinedTextureParameter::CombinedTextureParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
{
}

CombinedTextureParameter::~CombinedTextureParameter() 
{
    
}


void CombinedTextureParameter::updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8*) 
{
    descriptorSet->updateTexture(binding, data->getTexture(), sampler);
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
    sampler = buffer.getGraphics()->createSamplerState({});
}

void Serialization::save(ArchiveBuffer& buffer, const PShaderParameter& parameter)
{
    Serialization::save(buffer, parameter->getIdentifier());
    parameter->save(buffer);
}

void Serialization::load(ArchiveBuffer& buffer, PShaderParameter& parameter)
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