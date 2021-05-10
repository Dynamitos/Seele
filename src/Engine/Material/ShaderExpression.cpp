#include "ShaderExpression.h"
#include "Graphics/GraphicsResources.h"
#include "Asset/TextureAsset.h"

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

FloatParameter::FloatParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
{
}

FloatParameter::~FloatParameter() 
{
}

void FloatParameter::updateDescriptorSet(Gfx::PDescriptorSet, uint8* dst) 
{
    std::memcpy(dst + byteOffset, &data, sizeof(float));
}

VectorParameter::VectorParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
{
}

VectorParameter::~VectorParameter() 
{
}

void VectorParameter::updateDescriptorSet(Gfx::PDescriptorSet, uint8* dst) 
{
    std::memcpy(dst + byteOffset, &data, sizeof(Vector));
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

CombinedTextureParameter::CombinedTextureParameter(std::string name, uint32 byteOffset, uint32 binding) 
    : ShaderParameter(name, byteOffset, binding)
{
}

CombinedTextureParameter::~CombinedTextureParameter() 
{
    
}

void CombinedTextureParameter::updateDescriptorSet(Gfx::PDescriptorSet descriptorSet, uint8* dst) 
{
    descriptorSet->updateTexture(binding, data->getTexture(), sampler);
}