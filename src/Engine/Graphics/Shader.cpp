#include "Shader.h"
#include "Graphics/RenderPass/DepthPrepass.h"
#include "Graphics/RenderPass/BasePass.h"
#include <format>

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

void ShaderCompiler::registerRenderPass(std::string name, std::string mainFile, bool useMaterials, bool hasFragmentShader, std::string fragmentFile, bool useMeshShading, bool hasTaskShader, std::string taskFile)
{
	passes[name] = PassConfig{
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
	ShaderPermutation permutation;
	for (const auto& [name, pass] : passes)
	{
		std::memset(&permutation, 0, sizeof(ShaderPermutation));
		permutation.hasFragment = pass.hasFragmentShader;
		permutation.useMeshShading = pass.useMeshShading;
		permutation.hasTaskShader = pass.hasTaskShader;
		std::memcpy(permutation.vertexMeshFile, pass.mainFile.c_str(), sizeof(permutation.vertexMeshFile));
		if (pass.hasFragmentShader)
		{
			std::memcpy(permutation.fragmentFile, pass.fragmentFile.c_str(), sizeof(permutation.fragmentFile));
		}
		if (pass.hasTaskShader)
		{
			std::memcpy(permutation.taskFile, pass.taskFile.c_str(), sizeof(permutation.taskFile));
		}
		for (const auto& [vdName, vd] : vertexData)
		{
			std::memcpy(permutation.vertexDataName, vd->getTypeName().c_str(), sizeof(permutation.vertexDataName));
			if (pass.useMaterial)
			{
				for (const auto& [matName, mat] : materials)
				{
					std::memcpy(permutation.materialName, matName.c_str(), sizeof(permutation.materialName));
					createShaders(permutation);
				}
			}
			else
			{
				std::memset(permutation.materialName, 0, sizeof(permutation.materialName));
				createShaders(permutation);
			}
		}
	}
}

ShaderCollection& ShaderCompiler::createShaders(ShaderPermutation permutation)
{
	std::scoped_lock lock(shadersLock);
	ShaderCollection collection;

	ShaderCreateInfo createInfo;
	createInfo.typeParameter = { permutation.materialName, permutation.vertexDataName };
	createInfo.name = std::format("Material {0}", permutation.materialName);
	createInfo.additionalModules.add(permutation.materialName);
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
	PermutationId perm = PermutationId(permutation);
	shaders[perm] = std::move(collection);

	return shaders[perm];
}