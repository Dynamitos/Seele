#include "slang-compile.h"
#include <slang-com-ptr.h>
#include <slang.h>
#include "Containers/Array.h"
#include <fmt/core.h>
#include <iostream>

#define CHECK_RESULT(x) {SlangResult r = x; if(r != 0) {throw std::runtime_error(fmt::format("Error: {0}", r));}}
#define CHECK_DIAGNOSTICS() {if(diagnostics) {std::cout << (const char*)diagnostics->getBufferPointer() << std::endl; assert(false);}}

Slang::ComPtr<slang::IBlob> Seele::generateShader(const ShaderCreateInfo& createInfo, SlangCompileTarget target)
{
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
    vulkan.format = target;
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
    return kernelBlob;
}