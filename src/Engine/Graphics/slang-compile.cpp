#include "slang-compile.h"
#include <slang.h>
#include "Containers/Array.h"
#include <fmt/core.h>
#include <iostream>

#define CHECK_RESULT(x) {SlangResult r = x; if(r != 0) {throw std::runtime_error(fmt::format("Error: {0}", r));}}
#define CHECK_DIAGNOSTICS() {if(diagnostics) {std::cout << (const char*)diagnostics->getBufferPointer() << std::endl; assert(false);}}

Slang::ComPtr<slang::IBlob> Seele::generateShader(const ShaderCreateInfo& createInfo, SlangCompileTarget target, Map<std::string, uint32>& paramMapping)
{
    thread_local Slang::ComPtr<slang::IGlobalSession> globalSession;
    if(!globalSession)
    {
        slang::createGlobalSession(globalSession.writeRef());
    }
    slang::SessionDesc sessionDesc;
    sessionDesc.flags = 0;
    StaticArray<slang::CompilerOptionEntry, 2> option;
    option[0].name = slang::CompilerOptionName::DumpIntermediates;
    option[0].value = slang::CompilerOptionValue();
    option[0].value.kind = slang::CompilerOptionValueKind::Int;
    option[0].value.intValue0 = 1;
    option[1].name = slang::CompilerOptionName::EmitSpirvViaGLSL;
    option[1].value = slang::CompilerOptionValue();
    option[1].value.kind = slang::CompilerOptionValueKind::Int;
    option[1].value.intValue0 = 1;
    sessionDesc.compilerOptionEntries = option.data();
    sessionDesc.compilerOptionEntryCount = option.size();
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
    slang::TargetDesc targetDesc;
    targetDesc.profile = globalSession->findProfile("sm_6_6");
    targetDesc.format = target;
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
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
    slang::ProgramLayout* signature = specializedComponent->getLayout(0, diagnostics.writeRef());
    CHECK_DIAGNOSTICS();
    for(size_t i = 0; i < signature->getParameterCount(); ++i)
    {
        auto param = signature->getParameterByIndex(i);
        // workaround
        if (std::strcmp(param->getName(), "pVertexData") == 0)
        {
            paramMapping[param->getName()] = 1;
        }
        else if (std::strcmp(param->getName(), "pMaterial") == 0)
        {
            paramMapping[param->getName()] = 4;
        }
        else
        {
            paramMapping[param->getName()] = param->getBindingIndex();
        }
    }
    return kernelBlob;
}
