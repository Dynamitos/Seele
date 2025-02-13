#include "Shader.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Material/Material.h"
#include "ThreadPool.h"
#include <fmt/core.h>

using namespace Seele;
using namespace Seele::Gfx;

ShaderCompiler::ShaderCompiler(Gfx::PGraphics graphics) : graphics(graphics) {}

ShaderCompiler::~ShaderCompiler() {}

const ShaderCollection* ShaderCompiler::findShaders(PermutationId id) const { return &shaders[id]; }

void ShaderCompiler::registerMaterial(PMaterial material) {
    materials[material->getName()] = material;
    compile();
}

void ShaderCompiler::registerVertexData(VertexData* vd) {
    vertexData[vd->getTypeName()] = vd;
    compile();
}

void ShaderCompiler::registerRenderPass(std::string name, PassConfig config) {
    passes[name] = std::move(config);
    compile();
}

ShaderPermutation ShaderCompiler::getTemplate(std::string name) {
    std::scoped_lock lock(shadersLock);
    ShaderPermutation permutation;
    PassConfig& pass = passes[name];
    if (pass.useMeshShading) {
        permutation.setMeshFile(pass.mainFile);
    } else if (pass.rayTracing) {
        permutation.setRayTracingFile(pass.mainFile);
    } else {
        permutation.setVertexFile(pass.mainFile);
    }
    if (pass.hasFragmentShader) {
        permutation.setFragmentFile(pass.fragmentFile);
    }
    if (pass.hasTaskShader) {
        permutation.setTaskFile(pass.taskFile);
    }
    permutation.setVisibilityPass(pass.useVisibility);
    permutation.setDumpIntermediates(pass.dumpIntermediates);
    return permutation;
}

void ShaderCompiler::compile() {
    List<std::function<void()>> work;
    for (const auto& [name, pass] : passes) {
        for (const auto& [vdName, vd] : vertexData) {
            if (pass.useMaterial) {
                for (const auto& [matName, mat] : materials) {
                    for (int y = 0; y < 2; y++) {
                        work.add([=, this]() {
                            ShaderPermutation permutation = getTemplate(name);
                            permutation.setPositionOnly(false);
                            permutation.setDepthCulling(y);
                            permutation.setVertexData(vd->getTypeName());
                            OPipelineLayout layout = graphics->createPipelineLayout(pass.baseLayout->getName(), pass.baseLayout);
                            layout->addDescriptorLayout(vd->getVertexDataLayout());
                            layout->addDescriptorLayout(vd->getInstanceDataLayout());
                            permutation.setMaterial(mat->getName(), mat->getProfile());
                            createShaders(permutation, std::move(layout), name);
                        });
                    }
                }
            } else {
                for (int x = 0; x < 2; x++) {
                    for (int y = 0; y < 2; y++) {
                        work.add([=]() {
                            ShaderPermutation permutation = getTemplate(name);
                            permutation.setPositionOnly(x);
                            permutation.setDepthCulling(y);
                            permutation.setVertexData(vd->getTypeName());
                            OPipelineLayout layout = graphics->createPipelineLayout(pass.baseLayout->getName(), pass.baseLayout);
                            layout->addDescriptorLayout(vd->getVertexDataLayout());
                            layout->addDescriptorLayout(vd->getInstanceDataLayout());
                            createShaders(permutation, std::move(layout), name);
                        });
                    }
                }
            }
        }
    }
    getThreadPool().runAndWait(std::move(work));
}

void ShaderCompiler::createShaders(ShaderPermutation permutation, Gfx::OPipelineLayout layout, std::string debugName) {
    PermutationId perm = PermutationId(permutation);
    {
        std::scoped_lock lock(shadersLock);
        if (shaders.contains(perm))
            return;
    }
    ShaderCollection collection;
    collection.pipelineLayout = std::move(layout);

    ShaderCompilationInfo createInfo;
    createInfo.name = fmt::format("{0} Material {1}", debugName, permutation.materialName);
    createInfo.rootSignature = collection.pipelineLayout;
    if (std::strlen(permutation.materialName) > 0) {
        createInfo.modules.add(permutation.materialName);
        createInfo.defines["MATERIAL_FILE_NAME"] = permutation.materialName;
        //createInfo.typeParameter.add({"IBRDF", "Phong"});
    }
    if (permutation.positionOnly) {
        createInfo.defines["POS_ONLY"] = "1";
    }
    if (permutation.depthCulling) {
        createInfo.defines["DEPTH_CULLING"] = "1";
    }
    if (permutation.visibilityPass) {
        createInfo.defines["VISIBILITY"] = "1";
    }
    if (permutation.dumpIntermediates)
    {
        createInfo.dumpIntermediate = true;
    }
    //createInfo.typeParameter.add({Pair<const char*, const char*>("IVertexData", permutation.vertexDataName)});
    createInfo.modules.add(permutation.vertexDataName);
    //createInfo.dumpIntermediate = true;

    if (permutation.useMeshShading) {
        if (permutation.hasTaskShader) {
            createInfo.entryPoints.add({"taskMain", permutation.taskFile});
            createInfo.modules.add(permutation.taskFile);
        }
        createInfo.entryPoints.add({"meshMain", permutation.vertexMeshFile});
        createInfo.modules.add(permutation.vertexMeshFile);
    } else if (permutation.rayTracing) {
        createInfo.defines["RAY_TRACING"] = "1";
        createInfo.entryPoints = {{"closestHit", "ClosestHit"}};
        createInfo.modules.add(permutation.vertexMeshFile);
    } else {
        createInfo.entryPoints.add({"vertexMain", permutation.vertexMeshFile});
        createInfo.modules.add(permutation.vertexMeshFile);
    }
    if (permutation.hasFragment) {
        createInfo.entryPoints.add({"fragmentMain", permutation.fragmentFile});
        createInfo.modules.add(permutation.fragmentFile);
    }

    graphics->beginShaderCompilation(createInfo);
    uint32 shaderIndex = 0;
    if (permutation.useMeshShading) {
        if (permutation.hasTaskShader) {
            collection.taskShader = graphics->createTaskShader({shaderIndex++});
        }
        collection.meshShader = graphics->createMeshShader({shaderIndex++});
    } else if (permutation.rayTracing) {
        collection.callableShader = graphics->createClosestHitShader({shaderIndex++});
    } else {
        collection.vertexShader = graphics->createVertexShader({shaderIndex++});
    }
    if (permutation.hasFragment) {
        collection.fragmentShader = graphics->createFragmentShader({shaderIndex++});
    }
    collection.pipelineLayout->create();
    {
        std::scoped_lock lock(shadersLock);
        shaders[perm] = std::move(collection);
    }
}
