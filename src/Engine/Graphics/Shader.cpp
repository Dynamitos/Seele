#include "Shader.h"
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

const ShaderCollection* ShaderCompiler::findShaders(PermutationId id) const
{
	return &shaders[id];
}

void ShaderCompiler::registerMaterial(PMaterial material)
{
	materials[material->getName()] = material;
	compile();
}

void ShaderCompiler::registerVertexData(VertexData* vd)
{
	vertexData[vd->getTypeName()] = vd;
	compile();
}

void ShaderCompiler::registerRenderPass(Gfx::PPipelineLayout layout, std::string name, std::string mainFile, bool useMaterials, bool hasFragmentShader, std::string fragmentFile, bool useMeshShading, bool hasTaskShader, std::string taskFile)
{
	passes[name] = PassConfig{
        .baseLayout = layout,
		.taskFile = taskFile,
		.mainFile = mainFile,
		.fragmentFile = fragmentFile,
		.hasFragmentShader = hasFragmentShader,
		.useMeshShading = useMeshShading,
		.hasTaskShader = hasTaskShader,
		.useMaterial = useMaterials,
	};
	compile();
}


void ShaderCompiler::compile()
{
	for (const auto& [name, pass] : passes)
	{
        ShaderPermutation permutation;
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
		for (const auto& [vdName, vd] : vertexData)
		{
            permutation.setVertexData(vd->getTypeName());
			if (pass.useMaterial)
			{
				for (const auto& [matName, mat] : materials)
				{
                    OPipelineLayout layout = graphics->createPipelineLayout(pass.baseLayout->getName(), pass.baseLayout);
                    layout->addDescriptorLayout(vd->getVertexDataLayout());
                    layout->addDescriptorLayout(vd->getInstanceDataLayout());
                    layout->addDescriptorLayout(mat->getDescriptorLayout());
                    Map<std::string, uint32> mapping;
                    mapping["pLightCullingData"] = 1;
                    mapping["pMaterial"] = 2;
                    mapping["pViewParams"] = 3;
                    mapping["pVertexData"] = 4;
                    mapping["pScene"] = 5;
                    mapping["pLightEnv"] = 6;
                    layout->addMapping(mapping);
					permutation.setMaterial(mat->getName());
                    createShaders(permutation, std::move(layout));
				}
			}
			else
			{
                OPipelineLayout layout = graphics->createPipelineLayout(pass.baseLayout->getName(), pass.baseLayout);
                layout->addDescriptorLayout(vd->getVertexDataLayout());
                layout->addDescriptorLayout(vd->getInstanceDataLayout());
                Map<std::string, uint32> mapping;
                mapping["pViewParams"] = 1;
                mapping["pVertexData"] = 2;
                mapping["pScene"] = 3;
                layout->addMapping(mapping);
                createShaders(permutation, std::move(layout));
			}
		}
	}
}

void ShaderCompiler::createShaders(ShaderPermutation permutation, Gfx::OPipelineLayout layout)
{
	std::scoped_lock lock(shadersLock);
	PermutationId perm = PermutationId(permutation);
	if (shaders.contains(perm))
		return;
	ShaderCollection collection;
    collection.pipelineLayout = std::move(layout);

	ShaderCreateInfo createInfo;
	createInfo.name = fmt::format("Material {0}", permutation.materialName);
    createInfo.rootSignature = collection.pipelineLayout;
	if (std::strlen(permutation.materialName) > 0)
    {
        createInfo.additionalModules.add(permutation.materialName);
        createInfo.typeParameter.add(Pair<const char*, const char*>("IMaterial", permutation.materialName));
    }
	createInfo.typeParameter.add({ Pair<const char*, const char*>("IVertexData", permutation.vertexDataName) });
	createInfo.additionalModules.add(permutation.vertexDataName);
	createInfo.additionalModules.add(permutation.vertexMeshFile);
	if (permutation.hasFragment)
	{
		createInfo.additionalModules.add(permutation.fragmentFile);
	}
	if (permutation.hasTaskShader)
	{
		createInfo.additionalModules.add(permutation.taskFile);
	}

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
	shaders[perm] = std::move(collection);
}
