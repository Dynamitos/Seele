#include "Shader.h"
#include "Graphics/Initializer.h"
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
		permutation.hasFragment = pass.hasFragmentShader;
		permutation.useMeshShading = pass.useMeshShading;
		permutation.hasTaskShader = pass.hasTaskShader;
		std::strncpy(permutation.vertexMeshFile, pass.mainFile.c_str(), sizeof(permutation.vertexMeshFile));
		if (pass.hasFragmentShader)
		{
			std::strncpy(permutation.fragmentFile, pass.fragmentFile.c_str(), sizeof(permutation.fragmentFile));
		}
		if (pass.hasTaskShader)
		{
			std::strncpy(permutation.taskFile, pass.taskFile.c_str(), sizeof(permutation.taskFile));
		}
		for (const auto& [vdName, vd] : vertexData)
		{
			std::strncpy(permutation.vertexDataName, vd->getTypeName().c_str(), sizeof(permutation.vertexDataName));
			if (pass.useMaterial)
			{
				for (const auto& [matName, mat] : materials)
				{
					std::strncpy(permutation.materialName, matName.c_str(), sizeof(permutation.materialName));
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
	createInfo.typeParameter = { permutation.vertexDataName };
	createInfo.name = std::format("Material {0}", permutation.materialName);
	if (std::strlen(permutation.materialName) > 0)
	{
		createInfo.additionalModules.add(permutation.materialName);
		createInfo.typeParameter.add(permutation.materialName);
	}
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
	collection.vertexDeclaration = graphics->createVertexDeclaration(Array<VertexElement>());
	PermutationId perm = PermutationId(permutation);
	shaders[perm] = std::move(collection);

	return shaders[perm];
}