#include "Material.h"
#include "Window/WindowManager.h"
#include "MaterialInstance.h"
#include "Graphics/VertexShaderInput.h"
#include "Graphics/Graphics.h"

using namespace Seele;

Gfx::ShaderMap Material::shaderMap;
std::mutex Material::shaderMapLock;

Material::Material(Gfx::PGraphics graphics, 
        Array<PShaderParameter> parameter, 
        Gfx::PDescriptorLayout layout, 
        uint32 uniformDataSize, 
        uint32 uniformBinding, 
        std::string materialName, 
        Array<PShaderExpression> expressions, 
        MaterialNode brdf)
    : MaterialInterface(graphics, parameter, uniformDataSize, uniformBinding)
    , layout(layout)
    , materialName(materialName)
    , codeExpressions(expressions)
    , brdf(brdf)
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

void Material::save(ArchiveBuffer& buffer) const
{
    MaterialInterface::save(buffer);
    Serialization::save(buffer,materialName);
    Serialization::save(buffer, layout->getSetIndex());
    const auto& bindings = layout->getBindings();
    Serialization::save(buffer, bindings.size());
    for (const auto& binding : bindings)
    {
        Serialization::save(buffer, binding.binding);
        Serialization::save(buffer, binding.bindingFlags);
        Serialization::save(buffer, binding.descriptorCount);
        Serialization::save(buffer, binding.descriptorType);
        Serialization::save(buffer, binding.shaderStages);
    }
}

void Material::load(ArchiveBuffer& buffer)
{
    graphics = buffer.getGraphics();
    MaterialInterface::load(buffer);
    Serialization::load(buffer, materialName);
    uint32 setIndex;
    Serialization::load(buffer, setIndex);
    uint64 numBindings;
    Serialization::load(buffer, numBindings);
    layout = graphics->createDescriptorLayout();
    for (uint64 i = 0; i < numBindings; ++i)
    {
        uint32 binding;
        Serialization::load(buffer, binding);

        Gfx::SeDescriptorBindingFlags bindingFlags;
        Serialization::load(buffer, bindingFlags);

        uint32 descriptorCount;
        Serialization::load(buffer, descriptorCount);

        Gfx::SeDescriptorType descriptorType;
        Serialization::load(buffer, descriptorType);

        Gfx::SeShaderStageFlags shaderStages;
        Serialization::load(buffer, shaderStages);

        layout->addDescriptorBinding(binding, descriptorType, descriptorCount, bindingFlags, shaderStages);
    }
    layout->create();
}

void Material::compile()
{
    std::ofstream codeStream("./shaders/generated/"+materialName+".slang");
    codeStream << "import Material;\n";
    codeStream << "import BRDF;\n";
    codeStream << "import MaterialParameter;\n";
    codeStream << "struct " << materialName << " : IMaterial {\n";
    for(const auto& parameter : parameters)
    {
        parameter->generateDeclaration(codeStream);
    }
    codeStream << "\ttypedef " << brdf.profile << " BRDF;\n";
    codeStream << "\t" << brdf.profile << " prepare(MaterialFragmentParameter input) {\n";
    codeStream << "\t\t" << brdf.profile << " result;\n";
    Map<int32, std::string> varState;
    for(const auto& expr : codeExpressions)
    {
        codeStream << expr->evaluate(varState);
    }
    for(const auto& [name, exp] : brdf.variables)
    {
        codeStream << "\t\tresult." << name << " = " << varState[exp->key] << ";";
    }
    codeStream << "\t\treturn result;\n";
    codeStream << "\t}\n";
    codeStream << "};\n";
}

const Gfx::ShaderCollection* Material::getShaders(Gfx::RenderPassType renderPass, VertexInputType* vertexInput) const
{
    Gfx::ShaderPermutation permutation;
    permutation.passType = renderPass;
    std::string vertexInputName = vertexInput->getName();
    std::strncpy(permutation.materialName, materialName.c_str(), sizeof(permutation.materialName));
    std::strncpy(permutation.vertexInputName, vertexInputName.c_str(), sizeof(permutation.vertexInputName));
    return shaderMap.findShaders(Gfx::PermutationId(permutation));
}

Gfx::ShaderCollection& Material::createShaders(Gfx::PGraphics graphics, Gfx::RenderPassType renderPass, VertexInputType* vertexInput) 
{
    std::scoped_lock l(shaderMapLock);
    return shaderMap.createShaders(graphics, renderPass, this, vertexInput, false);
}
