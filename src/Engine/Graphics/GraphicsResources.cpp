#include "GraphicsResources.h"
#include "Graphics.h"
#include "RenderPass/DepthPrepass.h"
#include "RenderPass/BasePass.h"

using namespace Seele;
using namespace Seele::Gfx;

std::string getShaderNameFromRenderPassType(Gfx::RenderPassType type)
{
	switch (type)
	{
	case Gfx::RenderPassType::DepthPrepass:
		return "DepthPrepass.slang";
	case Gfx::RenderPassType::BasePass:
		return "ForwardPlus.slang";
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
	for(uint32 i = 0; i < shaders.size(); ++i)
	{
		if(shaders[i].id == id)
		{
			return &(shaders[i]);
		}
	}
	return nullptr;
}
ShaderCollection& ShaderMap::createShaders(
	PGraphics graphics,
	RenderPassType renderPass, 
	PMaterialAsset material, 
	VertexInputType* vertexInput,
	bool /*bPositionOnly*/)
{
	std::scoped_lock lock(shadersLock);
	ShaderCollection& collection = shaders.add();
	//collection.vertexDeclaration = bPositionOnly ? vertexInput->getPositionDeclaration() : vertexInput->getDeclaration();

	ShaderCreateInfo createInfo;
	createInfo.entryPoint = "vertexMain";
	createInfo.typeParameter = {material->getName().c_str()};
	createInfo.defines["VERTEX_INPUT_IMPORT"] = vertexInput->getShaderFilename();
	createInfo.defines["MATERIAL_IMPORT"] = material->getName().c_str();
	createInfo.defines["NUM_MATERIAL_TEXCOORDS"] = "1";
	createInfo.defines["USE_INSTANCING"] = "0";
	modifyRenderPassMacros(renderPass, createInfo.defines);
	createInfo.name = getShaderNameFromRenderPassType(renderPass) + " Material " + material->getName();
	
    std::ifstream codeStream("./shaders/" + getShaderNameFromRenderPassType(renderPass));
    
	createInfo.shaderCode.add(std::string(std::istreambuf_iterator<char>{codeStream}, {}));

	collection.vertexShader = graphics->createVertexShader(createInfo);

	if(renderPass != RenderPassType::DepthPrepass)
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
void DescriptorLayout::addDescriptorBinding(uint32 bindingIndex, SeDescriptorType type, uint32 arrayCount, SeDescriptorBindingFlags bindingFlags)
{
	if (descriptorBindings.size() <= bindingIndex)
	{
		descriptorBindings.resize(bindingIndex + 1);
	}
	DescriptorBinding& binding = descriptorBindings[bindingIndex];
	binding.binding = bindingIndex;
	binding.descriptorType = type;
	binding.descriptorCount = arrayCount;
	binding.bindingFlags = bindingFlags;
}

PDescriptorSet DescriptorLayout::allocateDescriptorSet()
{
	std::scoped_lock lock(allocatorLock);
	PDescriptorSet result;
	allocator->allocateDescriptorSet(result);
	return result;
}

void DescriptorLayout::reset() 
{
	std::scoped_lock lock(allocatorLock);
	allocator->reset();
}

void PipelineLayout::addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout)
{
	if (descriptorSetLayouts.size() <= setIndex)
	{
		descriptorSetLayouts.resize(setIndex + 1);
	}
	descriptorSetLayouts[setIndex] = layout;
	layout->setIndex = setIndex;
}

void PipelineLayout::addPushConstants(const SePushConstantRange &pushConstant)
{
	pushConstants.add(pushConstant);
}

QueueOwnedResource::QueueOwnedResource(QueueFamilyMapping mapping, QueueType startQueueType)
	: currentOwner(startQueueType)
	, mapping(mapping)
{
}

QueueOwnedResource::~QueueOwnedResource()
{
}

void QueueOwnedResource::transferOwnership(QueueType newOwner)
{
	if(mapping.needsTransfer(currentOwner, newOwner))
	{
		executeOwnershipBarrier(newOwner);
	}
	currentOwner = newOwner;
}

void QueueOwnedResource::pipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess, SePipelineStageFlags dstStage) 
{
	// maybe add some checks
	executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}

Buffer::Buffer(QueueFamilyMapping mapping, QueueType startQueue)
	: QueueOwnedResource(mapping, startQueue)
{
}

Buffer::~Buffer()
{
}

UniformBuffer::UniformBuffer(QueueFamilyMapping mapping, const BulkResourceData& resourceData)
	: Buffer(mapping, resourceData.owner)
	, contents(resourceData.size)
{
	if(resourceData.data != nullptr)
	{
		std::memcpy(contents.data(), resourceData.data, contents.size());
	}
}

UniformBuffer::~UniformBuffer()
{
}

bool UniformBuffer::updateContents(const BulkResourceData& resourceData) 
{
	assert(contents.size() == resourceData.size);
	if(std::memcmp(contents.data(), resourceData.data, contents.size()) == 0)
	{
		return false;
	}
	std::memcpy(contents.data(), resourceData.data, contents.size());
	return true;
}

StructuredBuffer::StructuredBuffer(QueueFamilyMapping mapping, const BulkResourceData& resourceData)
	: Buffer(mapping, resourceData.owner)
	, contents(resourceData.size)
{
}
StructuredBuffer::~StructuredBuffer()
{
}

bool StructuredBuffer::updateContents(const BulkResourceData& resourceData) 
{
	assert(contents.size() == resourceData.size);
	if(std::memcmp(contents.data(), resourceData.data, contents.size()) == 0)
	{
		return false;
	}
	std::memcpy(contents.data(), resourceData.data, contents.size());
	return true;
}
VertexBuffer::VertexBuffer(QueueFamilyMapping mapping, uint32 numVertices, uint32 vertexSize, QueueType startQueueType)
	: Buffer(mapping, startQueueType)
	, numVertices(numVertices)
	, vertexSize(vertexSize)
{
}
VertexBuffer::~VertexBuffer()
{
}
IndexBuffer::IndexBuffer(QueueFamilyMapping mapping, uint32 size, Gfx::SeIndexType indexType, QueueType startQueueType)
	: Buffer(mapping, startQueueType)
	, indexType(indexType)
{
	switch (indexType)
	{
	case SE_INDEX_TYPE_UINT16:
		numIndices = size / sizeof(uint16);
		break;
	case SE_INDEX_TYPE_UINT32:
		numIndices = size / sizeof(uint32);
	default:
		break;
	}
}
IndexBuffer::~IndexBuffer()
{
}
VertexStream::VertexStream()
{}
VertexStream::VertexStream(uint32 stride, uint32 offset, uint8 instanced)
	: stride(stride)
	, offset(offset)
	, instanced(instanced)
{
}
VertexStream::~VertexStream()
{
}
void VertexStream::addVertexElement(VertexElement element)
{
	vertexDescription.add(element);
}
const Array<VertexElement> VertexStream::getVertexDescriptions() const
{
	return vertexDescription;
}
VertexDeclaration::VertexDeclaration()
{
}
VertexDeclaration::~VertexDeclaration()
{
}

static std::mutex vertexDeclarationLock;
static Map<uint32, PVertexDeclaration> vertexDeclarationCache;

PVertexDeclaration VertexDeclaration::createDeclaration(PGraphics graphics, const Array<VertexElement>& elementList)
{
	std::scoped_lock lock(vertexDeclarationLock);
	boost::crc_32_type result;
	result.process_bytes(&elementList, sizeof(VertexElement) * elementList.size());
	uint32 key = result.checksum();

	auto found = vertexDeclarationCache[key];
	if(found == nullptr)
	{
		return found;
	}

	PVertexDeclaration newDeclaration = graphics->createVertexDeclaration(elementList);

	vertexDeclarationCache[key] = newDeclaration;
	return newDeclaration;
}

Texture::Texture(QueueFamilyMapping mapping, Gfx::QueueType startQueueType)
	: QueueOwnedResource(mapping, startQueueType)
{
}

Texture::~Texture()
{
}

Texture2D::Texture2D(QueueFamilyMapping mapping, Gfx::QueueType startQueueType)
	: Texture(mapping, startQueueType)
{	
}

Texture2D::~Texture2D()
{
}

RenderCommand::RenderCommand()
{
}

RenderCommand::~RenderCommand()
{
}

ComputeCommand::ComputeCommand() 
{
	
}

ComputeCommand::~ComputeCommand() 
{
	
}

RenderTargetLayout::RenderTargetLayout()
	: inputAttachments()
	, colorAttachments()
	, depthAttachment()
{
}

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, colorAttachments()
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
}

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment colorAttachment, PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
	colorAttachments.add(colorAttachment);
}
RenderTargetLayout::RenderTargetLayout(Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment)
	: inputAttachments()
	, colorAttachments(colorAttachments)
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
}
RenderTargetLayout::RenderTargetLayout(Array<PRenderTargetAttachment> inputAttachments, Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment)
	: inputAttachments(inputAttachments)
	, colorAttachments(colorAttachments)
	, depthAttachment(depthAttachment)
	, width(depthAttachment->getTexture()->getSizeX())
	, height(depthAttachment->getTexture()->getSizeY())
{
}

Window::Window(const WindowCreateInfo &createInfo)
	: sizeX(createInfo.width)
	, sizeY(createInfo.height)
	, bFullscreen(createInfo.bFullscreen)
	, title(createInfo.title)
	, pixelFormat(createInfo.pixelFormat)
	, samples(createInfo.numSamples)
{
}

Window::~Window()
{
}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo &viewportInfo)
	: sizeX(viewportInfo.sizeX)
	, sizeY(viewportInfo.sizeY)
	, offsetX(viewportInfo.offsetX)
	, offsetY(viewportInfo.offsetY)
	, owner(owner)
{
}

Viewport::~Viewport()
{
}

