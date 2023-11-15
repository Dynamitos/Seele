#include "Material.h"
#include "Window/WindowManager.h"
#include "MaterialInstance.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"

using namespace Seele;

Material::Material()
{
}

Material::Material(Gfx::PGraphics graphics,
        Gfx::ODescriptorLayout layout, 
        uint32 uniformDataSize, 
        uint32 uniformBinding, 
        std::string materialName, 
        Array<OShaderExpression> expressions,
        Array<std::string> parameter,
        MaterialNode brdf)
    : graphics(graphics)
    , uniformDataSize(uniformDataSize)
    , uniformBinding(uniformBinding)
    , instanceId(0)
    , layout(std::move(layout))
    , materialName(materialName)
    , codeExpressions(std::move(expressions))
    , parameters(std::move(parameter))
    , brdf(std::move(brdf))
{
}

Material::~Material()
{
}

OMaterialInstance Material::instantiate()
{
    return new MaterialInstance(
        instanceId++, 
        graphics, 
        codeExpressions,
        parameters, 
        uniformBinding, 
        uniformDataSize);
}

void Material::save(ArchiveBuffer& buffer) const
{
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
}

void Material::load(ArchiveBuffer& buffer)
{
    graphics = buffer.getGraphics();
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
        PShaderParameter handle = PShaderExpression(*codeExpressions.find([&parameter](const OShaderExpression& exp) {return exp->key == parameter; }));
        handle->generateDeclaration(codeStream);
    }
    codeStream << "\ttypedef " << brdf.profile << " BRDF;\n";
    codeStream << "\t" << brdf.profile << " prepare(MaterialParameter input) {\n";
    codeStream << "\t\t" << brdf.profile << " result;\n";
    Map<std::string, std::string> varState;
    // initialize variable state
    for(const auto& expr :codeExpressions)
    {
        codeStream << expr->evaluate(varState);
    }
    for(const auto& [name, exp] : brdf.variables)
    {
        codeStream << "\t\tresult." << name << " = " << varState[exp] << ";";
    }
    codeStream << "\t\treturn result;\n";
    codeStream << "\t}\n";
    codeStream << "};\n";
    graphics->getShaderCompiler()->registerMaterial(this);
}
