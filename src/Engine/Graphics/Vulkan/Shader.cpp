#include "Shader.h"
#include "Graphics.h"
#include "slang.h"
#include "slang-com-ptr.h"
#include "stdlib.h"
#include <format>

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

uint32 Seele::Vulkan::Shader::getShaderHash() const
{
    return hash;
}

#define CHECK_RESULT(x) {SlangResult r = x; if(r != 0) {throw std::runtime_error(std::format("Error: {0}", r));}}
#define CHECK_DIAGNOSTICS() {if(diagnostics) {std::cout << (const char*)diagnostics->getBufferPointer() << std::endl; assert(false);}}

void Shader::create(const ShaderCreateInfo& createInfo)
{
    entryPointName = createInfo.entryPoint;
    thread_local Slang::ComPtr<slang::IGlobalSession> globalSession;
    if(!globalSession)
    {
        slang::createGlobalSession(globalSession.writeRef());
    }
    slang::SessionDesc sessionDesc;
    sessionDesc.flags = 0;
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    Array<slang::PreprocessorMacroDesc> macros;
    for(const auto& [key, val] : createInfo.defines)
    {
        macros.add(slang::PreprocessorMacroDesc{
            .name = key,
            .value = val,
        });
    }
    sessionDesc.preprocessorMacroCount = macros.size();
    sessionDesc.preprocessorMacros = macros.data();
    slang::TargetDesc vulkan;
    vulkan.profile = globalSession->findProfile("sm_6_6");
    vulkan.format = SLANG_SPIRV;
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &vulkan;
    StaticArray<const char*, 3> searchPaths = {"shaders/", "shaders/lib/", "shaders/generated/"};
    sessionDesc.searchPaths = searchPaths.data();
    sessionDesc.searchPathCount = searchPaths.size();

    Slang::ComPtr<slang::ISession> session;
    CHECK_RESULT(globalSession->createSession(sessionDesc, session.writeRef()));

    Slang::ComPtr<slang::IBlob> diagnostics;
    Array<slang::IComponentType*> modules;
    Slang::ComPtr<slang::IEntryPoint> entrypoint;
    slang::IModule* mainModule = nullptr;
    for (const auto& moduleName : createInfo.additionalModules)
    {
        modules.add(session->loadModule(moduleName.c_str(), diagnostics.writeRef()));
        if (moduleName == createInfo.mainModule)
        {
            mainModule = (slang::IModule*)modules.back();
        }
        CHECK_DIAGNOSTICS();
    }

    CHECK_DIAGNOSTICS();

    mainModule->findEntryPointByName(createInfo.entryPoint.c_str(), entrypoint.writeRef());
    modules.add(entrypoint);

    Slang::ComPtr<slang::IComponentType> moduleComposition;
    session->createCompositeComponentType(modules.data(), modules.size(), moduleComposition.writeRef(), diagnostics.writeRef());

    CHECK_DIAGNOSTICS();
    
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    moduleComposition->link(linkedProgram.writeRef(), diagnostics.writeRef());

    CHECK_DIAGNOSTICS();

    slang::ProgramLayout* reflection = linkedProgram->getLayout(0, diagnostics.writeRef());

    CHECK_DIAGNOSTICS();

    Array<slang::SpecializationArg> specialization;
    for(const auto& [key, value] : createInfo.typeParameter)
    {
        specialization.add(slang::SpecializationArg::fromType(reflection->findTypeByName(value)));
    }
    Slang::ComPtr<slang::IComponentType> specializedComponent;
    linkedProgram->specialize(specialization.data(), specialization.size(), specializedComponent.writeRef(), diagnostics.writeRef());
    CHECK_DIAGNOSTICS();

    Slang::ComPtr<slang::IBlob> kernelBlob;
    specializedComponent->getEntryPointCode(
        0,
        0,
        kernelBlob.writeRef(),
        diagnostics.writeRef()
    );
    CHECK_DIAGNOSTICS();

    for (uint32 i = 0; i < reflection->getParameterCount(); ++i)
    {
        slang::VariableLayoutReflection* varLayout = reflection->getParameterByIndex(i);
        //std::cout << varLayout->getName() << " in space " << varLayout->getBindingSpace() << " index " << varLayout->getBindingIndex() << std::endl;
    }
    VkShaderModuleCreateInfo moduleInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = kernelBlob->getBufferSize(),
        .pCode = (uint32_t*)kernelBlob->getBufferPointer(),
    };
    VK_CHECK(vkCreateShaderModule(graphics->getDevice(), &moduleInfo, nullptr, &module));

    hash = CRC::Calculate(entryPointName.data(), entryPointName.size(), CRC::CRC_32());
    hash = CRC::Calculate(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), CRC::CRC_32(), hash);
}