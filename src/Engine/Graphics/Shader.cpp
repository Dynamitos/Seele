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
					permutation.setMaterial(mat->getName());
					createShaders(permutation);
				}
			}
			else
			{
				createShaders(permutation);
			}
		}
	}
}

void ShaderCompiler::createShaders(ShaderPermutation permutation)
{
	std::scoped_lock lock(shadersLock);
	PermutationId perm = PermutationId(permutation);
	if (shaders.contains(perm))
		return;
	ShaderCollection collection;

	ShaderCreateInfo createInfo;
	createInfo.name = std::format("Material {0}", permutation.materialName);
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
	collection.vertexDeclaration = graphics->createVertexDeclaration(Array<VertexElement>());
	shaders[perm] = std::move(collection);
}