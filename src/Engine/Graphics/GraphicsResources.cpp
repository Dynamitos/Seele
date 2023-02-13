#include "GraphicsResources.h"
#include "Graphics.h"
#include "RenderPass/DepthPrepass.h"
#include "RenderPass/BasePass.h"
#include "Material/Material.h"

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
	PMaterial material, 
	VertexInputType* vertexInput,
	bool /*bPositionOnly*/)
{
	std::scoped_lock lock(shadersLock);
	ShaderCollection& collection = shaders.add();
	//collection.vertexDeclaration = bPositionOnly ? vertexInput->getPositionDeclaration() : vertexInput->getDeclaration();

	ShaderCreateInfo createInfo;
	createInfo.entryPoint = "vertexMain";
	createInfo.typeParameter = {material->getName().c_str()};
	createInfo.defines["NUM_MATERIAL_TEXCOORDS"] = "1";
	createInfo.defines["USE_INSTANCING"] = "0";
	modifyRenderPassMacros(renderPass, createInfo.defines);
	createInfo.name = getShaderNameFromRenderPassType(renderPass) + " Material " + material->getName();
	
    std::ifstream codeStream("./shaders/" + getShaderNameFromRenderPassType(renderPass));
    
	createInfo.mainModule = getShaderNameFromRenderPassType(renderPass);
	createInfo.additionalModules.add(vertexInput->getShaderFilename());
	createInfo.additionalModules.add(material->getName());

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
void DescriptorLayout::addDescriptorBinding(uint32 bindingIndex, SeDescriptorType type, uint32 arrayCount, SeDescriptorBindingFlags bindingFlags, SeShaderStageFlags shaderStages)
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
	binding.shaderStages = shaderStages;
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

StructuredBuffer::StructuredBuffer(QueueFamilyMapping mapping, uint32 stride, uint32 numElements, const BulkResourceData& resourceData)
	: Buffer(mapping, resourceData.owner)
	, contents(resourceData.size)
	, stride(stride)
	, numElements(numElements)
{
	if(resourceData.data != nullptr)
	{
		std::memcpy(contents.data(), resourceData.data, resourceData.size);
	}
}
StructuredBuffer::~StructuredBuffer()
{
}

bool StructuredBuffer::updateContents(const BulkResourceData& resourceData) 
{
	assert(contents.size() >= resourceData.size);
	if(std::memcmp(contents.data(), resourceData.data, resourceData.size) == 0)
	{
		return false;
	}
	std::memcpy(contents.data(), resourceData.data, resourceData.size);
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
IndexBuffer::IndexBuffer(QueueFamilyMapping mapping, uint64 size, Gfx::SeIndexType indexType, QueueType startQueueType)
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

VertexDataManager::VertexDataManager(PGraphics graphics)
	: currentSize(8 * 1024 * 1024)
	, inUse(0)
	, graphics(graphics)
{
	VertexBufferCreateInfo defaultInfo = {
		.resourceData = {
			.size = currentSize,
			.data = nullptr,
		}
	};
	buffer = graphics->createVertexBuffer(defaultInfo);
}

VertexDataManager::~VertexDataManager()
{

}

VertexDataAllocation VertexDataManager::createVertexBuffer(const VertexBufferCreateInfo& vbInfo)
{
	VertexDataAllocation data = allocateData(vbInfo.resourceData.size);
	
	buffer->updateRegion(BulkResourceData{
		.size = data.size,
		.offset = data.offset,
		.data = vbInfo.resourceData.data
	});

	activeAllocations[data.offset] = data;

	return data;
}

void VertexDataManager::freeAllocation(VertexDataAllocation alloc)
{
    uint64 lowerBound = alloc.offset;
    uint64 upperBound = alloc.offset + alloc.size;
	bool joinedLower = false;
	uint64 lowerOffset = 0;

	//Join lower bound
	for (auto& [offset, freeAlloc] : freeRanges)
	{
		if (freeAlloc.offset <= lowerBound
		&& freeAlloc.offset + freeAlloc.size >= upperBound)
		{
			// allocation is already in a free region
			return;
		}
		if (freeAlloc.offset + freeAlloc.size == lowerBound)
		{
			//extend freeAlloc by the allocatedSize
			freeAlloc.size += alloc.size;
			joinedLower = true;
			lowerOffset = freeAlloc.offset;
			break;
		}
	}
	//Join upper bound
	auto foundAlloc = freeRanges.find(upperBound);
	if (foundAlloc != freeRanges.end())
	{
		// There is a free allocation ending where the new free one ends
		if (joinedLower)
		{
			// extend allocHandle by another foundAlloc->allocatedSize bytes
			freeRanges[lowerOffset].size += foundAlloc->value.size;
			freeRanges.erase(upperBound);
		}
		else
		{
			// set foundAlloc back by size amount
			freeRanges[upperBound].offset -= alloc.size;
			freeRanges[upperBound].size += alloc.size;
			// place back at correct offset
			freeRanges[freeRanges[upperBound].offset] = freeRanges[upperBound];
			// remove from offset map since key changes
			freeRanges.erase(upperBound);
		}
    }
	else
	{
		// No matching upper bound found
		if(!joinedLower)
		{
			// Lower bound also not joined, create new range
			freeRanges[alloc.offset] = alloc;
		}
	}
	activeAllocations.erase(alloc.offset);
    inUse -= alloc.size;
}


VertexDataAllocation VertexDataManager::allocateData(uint64 size)
{
	for (auto& [offset, freeAllocation] : freeRanges)
    {
        assert(offset == freeAllocation.offset);
        if (freeAllocation.size == size)
        {
            activeAllocations[offset] = freeAllocation;
            freeRanges.erase(offset);
            inUse += size;
            return freeAllocation;
        }
        else if (size < freeAllocation.size)
        {
            freeAllocation.size -= size;
            freeAllocation.offset += size;
            VertexDataAllocation subAlloc = VertexDataAllocation(size, offset);
            activeAllocations[offset] = subAlloc;
            freeRanges[freeAllocation.offset] = freeAllocation;
            freeRanges.erase(offset);
            inUse += size;
            return subAlloc;
        }
    }
	throw std::logic_error("TODO: expand buffer");
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
	uint32 key = CRC::Calculate(&elementList, sizeof(VertexElement) * elementList.size(), CRC::CRC_32());

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

Texture3D::Texture3D(QueueFamilyMapping mapping, Gfx::QueueType startQueueType)
	: Texture(mapping, startQueueType)
{	
}

Texture3D::~Texture3D()
{
}

TextureCube::TextureCube(QueueFamilyMapping mapping, Gfx::QueueType startQueueType)
	: Texture(mapping, startQueueType)
{	
}

TextureCube::~TextureCube()
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
	: windowState(createInfo)
{
}

Window::~Window()
{
}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo &viewportInfo)
	: sizeX(viewportInfo.dimensions.size.x)
	, sizeY(viewportInfo.dimensions.size.y)
	, offsetX(viewportInfo.dimensions.offset.x)
	, offsetY(viewportInfo.dimensions.offset.y)
	, fieldOfView(viewportInfo.fieldOfView)
	, owner(owner)
{
}

Viewport::~Viewport()
{
}

Matrix4 Viewport::getProjectionMatrix() const
{
	if(fieldOfView > 0.0f)
	{
		return glm::perspective(fieldOfView, sizeX / static_cast<float>(sizeY), 0.1f, 1000.0f);
	}
	else
	{
		return glm::ortho(0.0f, (float)sizeX, (float)sizeY, 0.0f);
	}
}

