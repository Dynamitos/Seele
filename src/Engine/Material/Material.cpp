#include "Material.h"
#include "Window/WindowManager.h"
#include "MaterialInstance.h"

using namespace Seele;

Gfx::ShaderMap Material::shaderMap;
std::mutex Material::shaderMapLock;

Material::Material(Array<PShaderParameter> parameter, Gfx::PDescriptorLayout layout, uint32 uniformDataSize, uint32 uniformBinding, std::string materialName)
    : MaterialInterface(parameter, uniformDataSize, uniformBinding)
    , layout(layout)
    , materialName(materialName)
{
}

Material::~Material()
{
}

Gfx::PDescriptorSet Material::createDescriptorSet()
{
    Gfx::PDescriptorSet descriptorSet = layout->allocateDescriptorSet();
    BulkResourceData uniformUpdate = {
        .size = uniformDataSize,
        .data = (uint8*)uniformData.data(),
    };
    for(auto param : parameters)
    {
        param->updateDescriptorSet(descriptorSet, uniformData.data());
    }
    if(uniformUpdate.size != 0)
    {
        uniformBuffer->updateContents(uniformUpdate);
        descriptorSet->updateBuffer(uniformBinding, uniformBuffer);
    }
    descriptorSet->writeChanges();
    return descriptorSet;
}

PMaterialInstance Material::instantiate()
{
    return new MaterialInstance(this);
}

const Gfx::ShaderCollection* Material::getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const
{
    Gfx::ShaderPermutation permutation;
    permutation.passType = renderPass;
    std::string vertexInputName = vertexInput->getName();
    std::memcpy(permutation.materialName, materialName.c_str(), sizeof(permutation.materialName));
    std::memcpy(permutation.vertexInputName, vertexInputName.c_str(), sizeof(permutation.vertexInputName));
    return shaderMap.findShaders(Gfx::PermutationId(permutation));
}

Gfx::ShaderCollection& Material::createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput) 
{
    std::scoped_lock l(shaderMapLock);
    return shaderMap.createShaders(graphics, renderPass, this, vertexInput, false);
}
