#include "VulkanShader.h"
#include "VulkanGraphics.h"
#include "VulkanDescriptorSets.h"
#include "slang.h"

using namespace slang;
using namespace Seele;
using namespace Seele::Vulkan;

Shader::Shader(PGraphics graphics, ShaderType shaderType, VkShaderStageFlags stage) 
    : graphics(graphics)
    , type(shaderType)
    , stage(stage)
{
}

Shader::~Shader() 
{
    if(module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(graphics->getDevice(), module, nullptr);
    }
}

Map<uint32, PDescriptorLayout> Shader::getDescriptorLayouts()
{
    return descriptorSets;
}

static SlangStage getStageFromShaderType(ShaderType type)
{
    switch (type)
    {
    case ShaderType::VERTEX:
        return SLANG_STAGE_VERTEX;
    case ShaderType::CONTROL:
        return SLANG_STAGE_HULL;
    case ShaderType::EVALUATION:
        return SLANG_STAGE_DOMAIN;
    case ShaderType::GEOMETRY:
        return SLANG_STAGE_GEOMETRY;
    case ShaderType::FRAGMENT:
        return SLANG_STAGE_PIXEL;
    default:
        return SLANG_STAGE_NONE;
    }
}

void Shader::create(const ShaderCreateInfo& createInfo)
{
    entryPointName = createInfo.entryPoint;
    static SlangSession* session = spCreateSession(NULL);

    SlangCompileRequest* request = spCreateCompileRequest(session);
    int targetIndex = spAddCodeGenTarget(request, SLANG_SPIRV);
    spSetTargetProfile(request, targetIndex, spFindProfile(session, "glsl_vk"));
    spSetDumpIntermediates(request, true);

    int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "");

    for(auto code : createInfo.shaderCode)
    {
        spAddTranslationUnitSourceString(
            request,
            translationUnitIndex,
            entryPointName.c_str(),
            code.data()
        );
    }
    spAddSearchPath(request, "shaders/lib/");

    spSetGlobalGenericArgs(request, createInfo.typeParameter.size(), createInfo.typeParameter.data());
    
    int entryPointIndex = spAddEntryPoint(request, translationUnitIndex, entryPointName.c_str(), getStageFromShaderType(type));
    if(spCompile(request))
    {
        char const* diagnostice = spGetDiagnosticOutput(request);
        std::cout << diagnostice << std::endl;
    }

    ShaderReflection* reflection = slang::ShaderReflection::get(request);
    
    uint32 parameterCount = reflection->getParameterCount();
    for(uint32 i = 0; i < parameterCount; ++i)
    {
        VariableLayoutReflection* parameter =
            reflection->getParameterByIndex(i);
        uint32 descriptorIndex = parameter->getBindingSpace();
        uint32 descriptorBinding = parameter->getBindingIndex();
        PDescriptorLayout& layout = descriptorSets[descriptorIndex];
        std::cout << parameter->getTypeLayout()->getName() << std::endl;
        //layout->addDescriptorBinding(descriptorBinding, parame)
     }

    size_t dataSize = 0;
    const void* data = spGetEntryPointCode(request, entryPointIndex, &dataSize);

    VkShaderModuleCreateInfo moduleInfo;
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.pNext = nullptr;
	moduleInfo.flags = 0;
	moduleInfo.codeSize = dataSize;
	moduleInfo.pCode = static_cast<const uint32*>(data);
	VK_CHECK(vkCreateShaderModule(graphics->getDevice(), &moduleInfo, nullptr, &module));
}