#include "Shader.h"

using namespace Seele;
using namespace Seele::Gfx;

std::string getShaderNameFromRenderPassType(Gfx::RenderPassType type)
{
	switch (type)
	{
	case Gfx::RenderPassType::DepthPrepass:
		return "DepthPrepass";
	case Gfx::RenderPassType::BasePass:
		return "ForwardPlus";
	default:
		return "";
	}
}
void modifyRenderPassMacros(Gfx::RenderPassType type, Map<const char*, const char*>& defines)
{
	switch (type)
	{
	case Gfx::RenderPassType::DepthPrepass:
		DepthPrepass::modifyRenderPassMacros(defines);
		break;
	case Gfx::RenderPassType::BasePass:
		BasePass::modifyRenderPassMacros(defines);
		break;
	}
}

ShaderMap::ShaderMap()
{
}

ShaderMap::~ShaderMap()
{
}

const ShaderCollection* ShaderMap::findShaders(PermutationId&& id) const
{
	for (uint32 i = 0; i < shaders.size(); ++i)
	{
		if (shaders[i].id == id)
		{
			return &(shaders[i]);
		}
	}
	return nullptr;
}
ShaderCollection& ShaderMap::createShaders(
	PGraphics graphics,
	RenderPassType renderPass,
	PMaterial material,
	VertexInputType* vertexInput,
	bool /*bPositionOnly*/)
{
	std::scoped_lock lock(shadersLock);
	ShaderCollection& collection = shaders.add();
	//collection.vertexDeclaration = bPositionOnly ? vertexInput->getPositionDeclaration() : vertexInput->getDeclaration();

	ShaderCreateInfo createInfo;
	createInfo.entryPoint = "vertexMain";
	createInfo.typeParameter = { material->getName().c_str() };
	createInfo.defines["NUM_MATERIAL_TEXCOORDS"] = "1";
	createInfo.defines["USE_INSTANCING"] = "0";
	modifyRenderPassMacros(renderPass, createInfo.defines);
	createInfo.name = getShaderNameFromRenderPassType(renderPass) + " Material " + material->getName();

	std::ifstream codeStream("./shaders/" + getShaderNameFromRenderPassType(renderPass));

	createInfo.mainModule = getShaderNameFromRenderPassType(renderPass);
	createInfo.additionalModules.add(vertexInput->getShaderFilename());
	createInfo.additionalModules.add(material->getName());

	collection.vertexShader = graphics->createVertexShader(createInfo);

	if (renderPass != RenderPassType::DepthPrepass)
	{
		createInfo.entryPoint = "fragmentMain";

		collection.fragmentShader = graphics->createFragmentShader(createInfo);
	}
	ShaderPermutation permutation;
	std::string materialName = material->getName();
	std::string vertexInputName = vertexInput->getName();
	permutation.passType = renderPass;
	std::memcpy(permutation.materialName, materialName.c_str(), sizeof(permutation.materialName));
	std::memcpy(permutation.vertexInputName, vertexInputName.c_str(), sizeof(permutation.vertexInputName));
	collection.id = PermutationId(permutation);

	return collection;
}