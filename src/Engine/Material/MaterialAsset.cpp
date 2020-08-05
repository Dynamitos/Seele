#include "MaterialAsset.h"
#include "Graphics/VertexShaderInput.h"

using namespace Seele;

Gfx::ShaderMap MaterialAsset::shaderMap;

MaterialAsset::MaterialAsset()
{
}

MaterialAsset::MaterialAsset(const std::string& directory, const std::string& name) 
    : Asset(directory, name)
{
}

MaterialAsset::MaterialAsset(const std::filesystem::path& fullPath) 
    : Asset(fullPath)
{
}

MaterialAsset::~MaterialAsset()
{
}

const Gfx::ShaderCollection* MaterialAsset::getShaders(Gfx::RenderPassType renderPass, PVertexShaderInput vertexInput) const
{
    Gfx::ShaderPermutation permutation;
    std::string materialName = getMaterialName();
    std::string vertexInputName = vertexInput->getName();
    std::memcpy(permutation.materialName, materialName.c_str(), sizeof(permutation.materialName));
    std::memcpy(permutation.materialName, vertexInputName.c_str(), sizeof(permutation.materialName));
    return shaderMap.findShaders(Gfx::PermutationId(permutation));
}

Gfx::ShaderCollection& MaterialAsset::createShaders(Gfx::RenderPassType renderPass, PVertexShaderInput vertexInput) 
{
    return Gfx::ShaderCollection();
}
