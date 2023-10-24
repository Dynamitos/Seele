#include "Material.h"
#include "Window/WindowManager.h"
#include "MaterialInstance.h"
#include "Graphics/Graphics.h"

using namespace Seele;


Material::Material(Gfx::PGraphics graphics, 
        Array<PShaderParameter> parameter, 
        Gfx::PDescriptorLayout layout, 
        uint32 uniformDataSize, 
        uint32 uniformBinding, 
        std::string materialName, 
        Array<PShaderExpression> expressions, 
        MaterialNode brdf)
    : graphics(graphics)
    , parameters(parameter)
    , uniformDataSize(uniformDataSize)
    , uniformBinding(uniformBinding)
    , layout(layout)
    , materialName(materialName)
    , codeExpressions(expressions)
    , brdf(brdf)
    , instanceId(0)
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
    Serialization::save(buffer, materialName);
    Serialization::save(buffer, layout->getSetIndex());
    Serialization::save(buffer, codeExpressions);
    Serialization::save(buffer, brdf);
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
    Serialization::load(buffer, codeExpressions);
    Serialization::load(buffer, brdf);
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
