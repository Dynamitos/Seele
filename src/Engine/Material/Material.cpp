#include "Material.h"
#include "Window/WindowManager.h"
#include "MaterialInstance.h"
#include "Graphics/Graphics.h"

using namespace Seele;

Material::Material()
{
}

Material::Material(Gfx::PGraphics graphics, 
        Array<OShaderParameter> parameter, 
        Gfx::ODescriptorLayout layout, 
        uint32 uniformDataSize, 
        uint32 uniformBinding, 
        std::string materialName, 
        Array<OShaderExpression> expressions, 
        MaterialNode brdf)
    : graphics(graphics)
    , parameters(std::move(parameter))
    , uniformDataSize(uniformDataSize)
    , uniformBinding(uniformBinding)
    , layout(std::move(layout))
    , materialName(materialName)
    , codeExpressions(std::move(expressions))
    , brdf(brdf)
    , instanceId(0)
{
}

Material::~Material()
{
}

PMaterialInstance Seele::Material::instantiate()
{
    return new MaterialInstance(instanceId++, graphics, this, layout, parameters, uniformBinding, uniformDataSize);
}

void Material::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, brdfName);
    Serialization::save(buffer, uniformDataSize);
    Serialization::save(buffer, uniformBinding);
    Serialization::save(buffer, instanceId);
    Serialization::save(buffer, materialName);
    Serialization::save(buffer, codeExpressions);
    Serialization::save(buffer, parameters);
    Serialization::save(buffer, brdf);
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
    Serialization::save(buffer, instances);
}

void Material::load(ArchiveBuffer& buffer)
{
    graphics = buffer.getGraphics();
    Serialization::load(buffer, brdfName);
    Serialization::load(buffer, uniformDataSize);
    Serialization::load(buffer, uniformBinding);
    Serialization::load(buffer, instanceId);
    Serialization::load(buffer, materialName);
    Serialization::load(buffer, codeExpressions);
    Serialization::load(buffer, parameters);
    Serialization::load(buffer, brdf);
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
    Serialization::load(buffer, instances);
    for (auto& instance : instances)
    {
        instance->setBaseMaterial(this);
    }
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
