#include "Shader.h"
#include "Graphics.h"
#include "slang.h"
#include "slang-com-ptr.h"
#include "stdlib.h"

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
    for(auto define : createInfo.defines)
    {
        macros.add(slang::PreprocessorMacroDesc{
            .name = define.key,
            .value = define.value
        });
    }
    sessionDesc.preprocessorMacroCount = macros.size();
    sessionDesc.preprocessorMacros = macros.data();
    slang::TargetDesc vulkan;
    vulkan.profile = globalSession->findProfile("glsl_vk");
    vulkan.format = SLANG_SPIRV;
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &vulkan;
    StaticArray<const char*, 3> searchPaths = {"shaders/", "shaders/lib/", "shaders/generated/"};
    sessionDesc.searchPaths = searchPaths.data();
    sessionDesc.searchPathCount = searchPaths.size();

    Slang::ComPtr<slang::ISession> session;
    globalSession->createSession(sessionDesc, session.writeRef());

    Slang::ComPtr<slang::IBlob> diagnostics;
    Array<slang::IComponentType*> modules;
    Slang::ComPtr<slang::IEntryPoint> entrypoint;
    
    for (auto moduleName : createInfo.additionalModules)
    {
        modules.add(session->loadModule(moduleName.c_str(), diagnostics.writeRef()));
        if(diagnostics)
        {
            std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
        }
    }
    slang::IModule* mainModule = session->loadModule(createInfo.mainModule.c_str(), diagnostics.writeRef());
    modules.add(mainModule);

    if(diagnostics)
    {
        std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
    }

    mainModule->findEntryPointByName(createInfo.entryPoint.c_str(), entrypoint.writeRef());
    modules.add(entrypoint);

    slang::IComponentType* moduleComposition;
    session->createCompositeComponentType(modules.data(), modules.size(), &moduleComposition, diagnostics.writeRef());

    if(diagnostics)
    {
        std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
    }

    /*for(auto typeParam : createInfo.typeParameter)
    {
        Slang::ComPtr<slang::ITypeConformance> typeConformance;
        session->createTypeConformanceComponentType(moduleComposition->getLayout()->findTypeByName(typeParam), moduleComposition->getLayout()->findTypeByName("IMaterial"), typeConformance.writeRef(), -1, diagnostics.writeRef());
        modules.add(typeConformance);
        if(diagnostics)
        {
            std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
        }
    }

    Slang::ComPtr<slang::IComponentType> conformingModule;
    session->createCompositeComponentType(modules.data(), modules.size(), conformingModule.writeRef(), diagnostics.writeRef());
*/
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    moduleComposition->link(linkedProgram.writeRef(), diagnostics.writeRef());
    if(diagnostics)
    {
        std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
    }

    slang::ProgramLayout* reflection = linkedProgram->getLayout(0, diagnostics.writeRef());
    if(diagnostics)
    {
        std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
    }

    Array<slang::SpecializationArg> specialization;
    for(auto typeArg : createInfo.typeParameter)
    {
        specialization.add(slang::SpecializationArg::fromType(reflection->findTypeByName(typeArg)));
    }
    Slang::ComPtr<slang::IComponentType> specializedComponent;
    linkedProgram->specialize(specialization.data(), specialization.size(), specializedComponent.writeRef(), diagnostics.writeRef());
    if(diagnostics)
    {
        std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
    }
    Slang::ComPtr<slang::IBlob> kernelBlob;
    specializedComponent->getEntryPointCode(
        0,
        0,
        kernelBlob.writeRef(),
        diagnostics.writeRef()
    );
    if(diagnostics)
    {
        std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
    }

    VkShaderModuleCreateInfo moduleInfo;
	moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.pNext = nullptr;
	moduleInfo.flags = 0;
	moduleInfo.codeSize = kernelBlob->getBufferSize();
	moduleInfo.pCode = (uint32_t*)kernelBlob->getBufferPointer();
	VK_CHECK(vkCreateShaderModule(graphics->getDevice(), &moduleInfo, nullptr, &module));

    hash = CRC::Calculate(entryPointName.data(), entryPointName.size(), CRC::CRC_32());
    hash = CRC::Calculate(kernelBlob->getBufferPointer(), kernelBlob->getBufferSize(), CRC::CRC_32(), hash);
}