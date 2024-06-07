#include "Shader.h"
#include "ThreadPool.h"
#include "Graphics/Initializer.h"
#include "Graphics/RenderPass/DepthPrepass.h"
#include "Graphics/RenderPass/BasePass.h"
#include <fmt/core.h>

using namespace Seele;
using namespace Seele::Gfx;

ShaderCompiler::ShaderCompiler(Gfx::PGraphics graphics)
    : graphics(graphics)
{
}

ShaderCompiler::~ShaderCompiler()
{
}

const ShaderCollection *ShaderCompiler::findShaders(PermutationId id) const
{
    return &shaders[id];
}

void ShaderCompiler::registerMaterial(PMaterial material)
{
    materials[material->getName()] = material;
    compile();
}

void ShaderCompiler::registerVertexData(VertexData *vd)
{
    vertexData[vd->getTypeName()] = vd;
    compile();
}

void ShaderCompiler::registerRenderPass(std::string name, PassConfig config)
{
    passes[name] = std::move(config);
    compile();
}

ShaderPermutation ShaderCompiler::getTemplate(std::string name)
{
    std::scoped_lock lock(shadersLock);
    ShaderPermutation permutation;
    PassConfig &pass = passes[name];
    if (pass.useMeshShading)
    {
        permutation.setMeshFile(pass.mainFile);
    }
    else
    {
        permutation.setVertexFile(pass.mainFile);
    }
    if (pass.hasFragmentShader)
    {
        permutation.setFragmentFile(pass.fragmentFile);
    }
    if (pass.hasTaskShader)
    {
        permutation.setTaskFile(pass.taskFile);
    }
    permutation.setVisibilityPass(pass.useVisibility);
    return permutation;
}

void ShaderCompiler::compile()
{
    List<std::function<void()>> work;
    for (const auto &[name, pass] : passes)
    {
        for (const auto &[vdName, vd] : vertexData)
        {
            if (pass.useMaterial)
            {
                for (const auto &[matName, mat] : materials)
                {
                    for (int x = 0; x < 2; x++)
                    {
                        for (int y = 0; y < 2; y++)
                        {
                            work.add([=]()
                                     {
							ShaderPermutation permutation = getTemplate(name);
							permutation.setPositionOnly(x);
							permutation.setViewCulling(y);
                            permutation.setVertexData(vd->getTypeName());
                            OPipelineLayout layout = graphics->createPipelineLayout(pass.baseLayout->getName(), pass.baseLayout);
							layout->addDescriptorLayout(vd->getVertexDataLayout());
							layout->addDescriptorLayout(vd->getInstanceDataLayout());
							layout->addDescriptorLayout(mat->getDescriptorLayout());
							permutation.setMaterial(mat->getName());
							createShaders(permutation, std::move(layout)); });
                        }
                    }
                }
            }
            else
            {
                for (int x = 0; x < 2; x++)
                {
                    for (int y = 0; y < 2; y++)
                    {
                        work.add([=]()
                                 {
						ShaderPermutation permutation = getTemplate(name);
						permutation.setPositionOnly(x);
						permutation.setViewCulling(y);
                        permutation.setVertexData(vd->getTypeName());
                        OPipelineLayout layout = graphics->createPipelineLayout(pass.baseLayout->getName(), pass.baseLayout);
						layout->addDescriptorLayout(vd->getVertexDataLayout());
						layout->addDescriptorLayout(vd->getInstanceDataLayout());
						createShaders(permutation, std::move(layout)); });
                    }
                }
            }
        }
    }
    getThreadPool().runAndWait(std::move(work));
}

void ShaderCompiler::createShaders(ShaderPermutation permutation, Gfx::OPipelineLayout layout)
{
    PermutationId perm = PermutationId(permutation);
    {
        std::scoped_lock lock(shadersLock);
        if (shaders.contains(perm))
            return;
    }
    ShaderCollection collection;
    collection.pipelineLayout = std::move(layout);

    ShaderCreateInfo createInfo;
    createInfo.name = fmt::format("Material {0}", permutation.materialName);
    createInfo.rootSignature = collection.pipelineLayout;
    if (std::strlen(permutation.materialName) > 0)
    {
        createInfo.additionalModules.add(permutation.materialName);
        createInfo.defines["MATERIAL_FILE_NAME"] = permutation.materialName;
    }
    if (permutation.positionOnly)
    {
        createInfo.defines["POS_ONLY"] = "1";
    }
    if (permutation.viewCulling)
    {
        createInfo.defines["VIEW_CULLING"] = "1";
    }
    if (permutation.visibilityPass)
    {
        createInfo.defines["VISIBILITY"] = "1";
    }
    createInfo.typeParameter.add({Pair<const char *, const char *>("IVertexData", permutation.vertexDataName)});
    createInfo.additionalModules.add(permutation.vertexDataName);

    if (permutation.useMeshShading)
    {
        if (permutation.hasTaskShader)
        {
            createInfo.mainModule = permutation.taskFile;
            createInfo.entryPoint = "taskMain";
            collection.taskShader = graphics->createTaskShader(createInfo);
        }
        createInfo.mainModule = permutation.vertexMeshFile;
        createInfo.entryPoint = "meshMain";
        collection.meshShader = graphics->createMeshShader(createInfo);
    }
    else
    {
        createInfo.mainModule = permutation.vertexMeshFile;
        createInfo.entryPoint = "vertexMain";
        collection.vertexShader = graphics->createVertexShader(createInfo);
    }

    if (permutation.hasFragment)
    {
        createInfo.mainModule = permutation.fragmentFile;
        createInfo.entryPoint = "fragmentMain";
        collection.fragmentShader = graphics->createFragmentShader(createInfo);
    }
    collection.pipelineLayout->create();
    {
        std::scoped_lock lock(shadersLock);
        shaders[perm] = std::move(collection);
    }
}
