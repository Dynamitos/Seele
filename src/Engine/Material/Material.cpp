#include "Material.h"
#include "Graphics/Enums.h"
#include "Serialization/Serialization.h"
#include "Window/WindowManager.h"
#include "MaterialInstance.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include <fstream>

using namespace Seele;

std::atomic_uint64_t Material::materialIdCounter = 0;
Array<PMaterial> Material::materials;

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
    , materialId(materialIdCounter++)
{
    materials.add(this);
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
    Serialization::save(buffer, layout->getName());
    const auto& bindings = layout->getBindings();
    Serialization::save(buffer, bindings.size());
    for (const auto& binding : bindings)
    {
        Serialization::save(buffer, binding.binding);
        Serialization::save(buffer, binding.descriptorType);
        Serialization::save(buffer, binding.textureType);
        Serialization::save(buffer, binding.descriptorCount);
        Serialization::save(buffer, binding.bindingFlags);
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
    std::string descriptorName;
    Serialization::load(buffer, descriptorName);
    uint64 numBindings;
    Serialization::load(buffer, numBindings);
    layout = graphics->createDescriptorLayout(descriptorName);
    for (uint64 i = 0; i < numBindings; ++i)
    {
        uint32 binding;
        Serialization::load(buffer, binding);

        Gfx::SeDescriptorType descriptorType;
        Serialization::load(buffer, descriptorType);

        Gfx::SeImageViewType textureType;
        Serialization::load(buffer, textureType);

        uint32 descriptorCount;
        Serialization::load(buffer, descriptorCount);

        Gfx::SeDescriptorBindingFlags bindingFlags;
        Serialization::load(buffer, bindingFlags);

        Gfx::SeShaderStageFlags shaderStages;
        Serialization::load(buffer, shaderStages);

        layout->addDescriptorBinding(Gfx::DescriptorBinding{.binding = binding, .descriptorType = descriptorType, .textureType = textureType, .descriptorCount = descriptorCount, .bindingFlags = bindingFlags, .shaderStages = shaderStages,});
    }
    layout->create();
    materialId = materialIdCounter++;
}

void Material::compile()
{
    std::ofstream codeStream("./shaders/generated/"+materialName+".slang");
    codeStream << "import BRDF;\n";
    codeStream << "import MaterialParameter;\n";
    codeStream << "struct " << materialName << "{\n";
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
        codeStream << "\t\t" << expr->evaluate(varState);
    }
    for(const auto& [name, exp] : brdf.variables)
    {
        codeStream << "\t\tresult." << name << " = " << varState[exp] << ";" << std::endl;
    }
    codeStream << "\t\treturn result;\n";
    codeStream << "\t}\n";
    codeStream << "};\n";
    codeStream << "layout(set=4)";
    codeStream << "ParameterBlock<" << materialName << "> pMaterial;";
    graphics->getShaderCompiler()->registerMaterial(this);
}
