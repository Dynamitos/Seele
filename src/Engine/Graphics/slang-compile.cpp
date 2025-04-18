#include "slang-compile.h"
#include "Containers/Array.h"
#include "Graphics/Descriptor.h"
#include <fmt/core.h>
#include <iostream>
#include <slang.h>

using namespace Seele;

#define CHECK_RESULT(x)                                                                                                                    \
    {                                                                                                                                      \
        SlangResult r = x;                                                                                                                 \
        if (r != 0) {                                                                                                                      \
            throw std::runtime_error(fmt::format("Error: {0}", r));                                                                        \
        }                                                                                                                                  \
    }
#define CHECK_DIAGNOSTICS()                                                                                                                \
    {                                                                                                                                      \
        if (diagnostics) {                                                                                                                 \
            std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;                                                        \
            abort();                                                                                                                       \
        }                                                                                                                                  \
    }

thread_local Slang::ComPtr<slang::IGlobalSession> globalSession;
thread_local Slang::ComPtr<slang::IComponentType> specializedComponent;
thread_local Slang::ComPtr<slang::ISession> session;
thread_local Array<std::string> entryPoints;

void Seele::beginCompilation(const ShaderCompilationInfo& info, SlangCompileTarget target, Gfx::PPipelineLayout layout) {
    if (!globalSession) {
        slang::createGlobalSession(globalSession.writeRef());
    }
    slang::SessionDesc sessionDesc;
    sessionDesc.flags = 0;
    Array<slang::CompilerOptionEntry> option = {
        {
            .name = slang::CompilerOptionName::LineDirectiveMode,
            .value =
                {
                    .kind = slang::CompilerOptionValueKind::Int,
                    .intValue0 = SLANG_LINE_DIRECTIVE_MODE_NONE,
                },
        },
        {
            .name = slang::CompilerOptionName::EmitSpirvViaGLSL,
            .value =
                {
                    .kind = slang::CompilerOptionValueKind::Int,
                    .intValue0 = 1,
                },
        },
        {
            .name = slang::CompilerOptionName::DebugInformation,
            .value =
                {
                    .kind = slang::CompilerOptionValueKind::Int,
                    .intValue0 = SLANG_DEBUG_INFO_LEVEL_STANDARD,
                },
        },
        {
            .name = slang::CompilerOptionName::DebugInformationFormat,
            .value =
                {
                    .kind = slang::CompilerOptionValueKind::Int,
                    .intValue0 = SLANG_DEBUG_INFO_FORMAT_PDB,
                },
        },
        {
            .name = slang::CompilerOptionName::DumpIntermediates,
            .value =
                {
                    .kind = slang::CompilerOptionValueKind::Int,
                    .intValue0 = info.dumpIntermediate,
                },
        },
    };

    sessionDesc.compilerOptionEntries = option.data();
    sessionDesc.compilerOptionEntryCount = (uint32)option.size();
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    Array<slang::PreprocessorMacroDesc> macros;
    for (const auto& [key, val] : info.defines) {
        macros.add(slang::PreprocessorMacroDesc{
            .name = key,
            .value = val,
        });
    }
    sessionDesc.preprocessorMacroCount = macros.size();
    sessionDesc.preprocessorMacros = macros.data();
    slang::TargetDesc targetDesc;
    targetDesc.profile = globalSession->findProfile("spv_1_4");
    targetDesc.format = target;
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    StaticArray<const char*, 6> searchPaths = {"shaders/", "shaders/lib/", "shaders/raytracing/", "shaders/terrain", "shaders/game", "shaders/generated/"};
    sessionDesc.searchPaths = searchPaths.data();
    sessionDesc.searchPathCount = searchPaths.size();

    Slang::ComPtr<slang::IBlob> diagnostics;

    CHECK_RESULT(globalSession->createSession(sessionDesc, session.writeRef()));
    Array<slang::IComponentType*> components;
    Map<std::string, slang::IModule*> moduleMap;
    for (const auto& moduleName : info.modules) {
        slang::IModule* loaded = session->loadModule(moduleName.c_str(), diagnostics.writeRef());
        CHECK_DIAGNOSTICS();
        components.add(loaded);
        moduleMap[moduleName] = loaded;
    }
    entryPoints.clear();
    for (const auto& [name, mod] : info.entryPoints) {
        entryPoints.add(name);
        slang::IEntryPoint* entry;
        CHECK_RESULT(moduleMap[mod]->findEntryPointByName(name.c_str(), &entry));
        components.add(entry);
    }

    Slang::ComPtr<slang::IComponentType> moduleComposition;
    session->createCompositeComponentType(components.data(), components.size(), moduleComposition.writeRef(), diagnostics.writeRef());

    CHECK_DIAGNOSTICS();

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    moduleComposition->link(linkedProgram.writeRef(), diagnostics.writeRef());

    CHECK_DIAGNOSTICS();

    slang::ProgramLayout* reflection = linkedProgram->getLayout(0, diagnostics.writeRef());

    CHECK_DIAGNOSTICS();

    Array<slang::SpecializationArg> specialization;
    for (const auto& [key, value] : info.typeParameter) {
        specialization.add(slang::SpecializationArg::fromType(reflection->findTypeByName(value)));
    }
    linkedProgram->specialize(specialization.data(), specialization.size(), specializedComponent.writeRef(), diagnostics.writeRef());
    CHECK_DIAGNOSTICS();

    slang::ProgramLayout* signature = specializedComponent->getLayout(0, diagnostics.writeRef());
    CHECK_DIAGNOSTICS();
    for (uint32 i = 0; i < signature->getParameterCount(); ++i) {
        auto param = signature->getParameterByIndex(i);
        layout->addMapping(param->getName(), param->getBindingIndex());
    }

    // workaround
    if (info.name == "RayGenMiss")
    {
        layout->addMapping("pScene", 2);
        layout->addMapping("pLightEnv", 3);
        layout->addMapping("pResources", 4);
        layout->addMapping("pRayTracingParams", 5);
    }
    layout->addMapping("pVertexData", 1);
    // layout->addMapping("pWaterMaterial", 1);
}

Pair<Slang::ComPtr<slang::IBlob>, std::string> Seele::generateShader(const ShaderCreateInfo& createInfo) {
    Slang::ComPtr<slang::IBlob> diagnostics;
    Slang::ComPtr<slang::IBlob> kernelBlob;
    specializedComponent->getEntryPointCode(createInfo.entryPointIndex, 0, kernelBlob.writeRef(), diagnostics.writeRef());
    CHECK_DIAGNOSTICS();
    return {kernelBlob, entryPoints[createInfo.entryPointIndex]};
}
