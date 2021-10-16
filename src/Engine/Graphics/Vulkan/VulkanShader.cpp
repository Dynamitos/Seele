#include "VulkanShader.h"
#include "VulkanGraphics.h"
#include "VulkanDescriptorSets.h"
#include "slang.h"
#include "stdlib.h"

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

uint32 Seele::Vulkan::Shader::getShaderHash() const
{
    return hash;
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
    static SlangSession* session = spCreateSession(nullptr);

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
    for(auto define : createInfo.defines)
    {
        spAddPreprocessorDefine(request, define.key, define.value);
    }
    spAddSearchPath(request, "shaders/lib/");
    spAddSearchPath(request, "shaders/generated/");

    spSetGlobalGenericArgs(request, (int)createInfo.typeParameter.size(), createInfo.typeParameter.data());
    
    int entryPointIndex = spAddEntryPoint(request, translationUnitIndex, entryPointName.c_str(), getStageFromShaderType(type));
    if(spCompile(request))
    {
        char const* diagnostics = spGetDiagnosticOutput(request);
        std::cout << "Compile error for shader " << createInfo.name << std::endl;
        std::cout << diagnostics << std::endl;
        std::cout << "Defines: " << std::endl;
        for(auto define : createInfo.defines)
        {
            std::cout << define.key << ": " << define.value << std::endl;
        }
        std::cout << "For shader code: " << std::endl;
        for(auto code : createInfo.shaderCode)
        {
            std::cout << code << std::endl;
        }
        return;
    }
    size_t dataSize = 0;
    const uint32* data = reinterpret_cast<const uint32*>(spGetEntryPointCode(request, entryPointIndex, &dataSize));

    VkShaderModuleCreateInfo moduleInfo;
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.pNext = nullptr;
	moduleInfo.flags = 0;
	moduleInfo.codeSize = dataSize;
	moduleInfo.pCode = data;
	VK_CHECK(vkCreateShaderModule(graphics->getDevice(), &moduleInfo, nullptr, &module));

    boost::crc_32_type result;
    result.process_bytes(entryPointName.data(), entryPointName.size());
    result.process_bytes(data, dataSize);
    hash = result.checksum();
}