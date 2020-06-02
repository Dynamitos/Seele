#include "GraphicsResources.h"
#include "Material/MaterialInstance.h"
#include "Graphics.h"

using namespace Seele;
using namespace Seele::Gfx;

void DescriptorLayout::addDescriptorBinding(uint32 bindingIndex, SeDescriptorType type, uint32 arrayCount)
{
	if (descriptorBindings.size() <= bindingIndex)
	{
		descriptorBindings.resize(bindingIndex + 1);
	}
	DescriptorBinding binding;
	binding.binding = bindingIndex;
	binding.descriptorType = type;
	binding.descriptorCount = arrayCount;
	descriptorBindings[bindingIndex] = binding;
}

PDescriptorSet DescriptorLayout::allocatedDescriptorSet()
{
	PDescriptorSet result;
	allocator->allocateDescriptorSet(result);
	return result;
}

void PipelineLayout::addDescriptorLayout(uint32 setIndex, PDescriptorLayout layout)
{
	if (descriptorSetLayouts.size() <= setIndex)
	{
		descriptorSetLayouts.resize(setIndex + 1);
	}
	if (descriptorSetLayouts[setIndex] != nullptr)
	{
		auto &thisBindings = descriptorSetLayouts[setIndex]->descriptorBindings;
		auto &otherBindings = layout->descriptorBindings;
		thisBindings.resize(otherBindings.size());
		for (size_t i = 0; i < otherBindings.size(); ++i)
		{
			if (otherBindings[i].descriptorType != SE_DESCRIPTOR_TYPE_MAX_ENUM)
			{
				assert(thisBindings[i].descriptorType != SE_DESCRIPTOR_TYPE_MAX_ENUM ? thisBindings[i].descriptorType == otherBindings[i].descriptorType : true);
				thisBindings[i] = otherBindings[i];
			}
		}
	}
	else
	{
		descriptorSetLayouts[setIndex] = layout;
		layout->setIndex = setIndex;
	}
}

void PipelineLayout::addPushConstants(const SePushConstantRange &pushConstant)
{
	pushConstants.add(pushConstant);
}

QueueOwnedResource::QueueOwnedResource(PGraphics graphics, QueueType startQueueType)
	: graphics(graphics)
	, currentOwner(startQueueType)
{
}

QueueOwnedResource::~QueueOwnedResource()
{
}

void QueueOwnedResource::transferOwnership(QueueType newOwner)
{
	if(graphics->getFamilyMapping().needsTransfer(currentOwner, newOwner))
	{
		executeOwnershipBarrier(newOwner);
		currentOwner = newOwner;
	}
}

Buffer::Buffer(PGraphics graphics, QueueType startQueue)
	: QueueOwnedResource(graphics, startQueue)
{
}

Buffer::~Buffer()
{
}

UniformBuffer::UniformBuffer(PGraphics graphics, QueueType startQueueType)
	: Buffer(graphics, startQueueType)
{
}

UniformBuffer::~UniformBuffer()
{
}
StructuredBuffer::StructuredBuffer(PGraphics graphics, QueueType startQueueType)
	: Buffer(graphics, startQueueType)
{
}
StructuredBuffer::~StructuredBuffer()
{
}
VertexBuffer::VertexBuffer(PGraphics graphics, uint32 numVertices, uint32 vertexSize, QueueType startQueueType)
	: Buffer(graphics, startQueueType)
	, numVertices(numVertices), vertexSize(vertexSize)
{
}
VertexBuffer::~VertexBuffer()
{
}
IndexBuffer::IndexBuffer(PGraphics graphics, uint32 size, Gfx::SeIndexType indexType, QueueType startQueueType)
	: Buffer(graphics, startQueueType)
	, indexType(indexType)
{
	switch (indexType)
	{
	case SE_INDEX_TYPE_UINT16:
		numIndices = size / 16;
		break;
	case SE_INDEX_TYPE_UINT32:
		numIndices = size / 32;
	default:
		break;
	}
}
IndexBuffer::~IndexBuffer()
{
}
VertexStream::VertexStream(uint32 stride, uint8 instanced)
	: stride(stride), instanced(instanced)
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
uint32 VertexDeclaration::addVertexStream(const VertexStream &element)
{
	uint32 currIndex = vertexStreams.size();
	vertexStreams.add(element);
	return currIndex;
}
const Array<VertexStream> &VertexDeclaration::getVertexStreams() const
{
	return vertexStreams;
}

Texture::Texture(PGraphics graphics, Gfx::QueueType startQueueType)
	: QueueOwnedResource(graphics, startQueueType)
{
}

Texture::~Texture()
{
}

Texture2D::Texture2D(PGraphics graphics, Gfx::QueueType startQueueType)
	: Texture(graphics, startQueueType)
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

RenderTargetLayout::RenderTargetLayout()
	: inputAttachments(), colorAttachments(), depthAttachment()
{
}

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment depthAttachment)
	: inputAttachments(), colorAttachments(), depthAttachment(depthAttachment)
{
}

RenderTargetLayout::RenderTargetLayout(PRenderTargetAttachment colorAttachment, PRenderTargetAttachment depthAttachment)
	: inputAttachments(), depthAttachment(depthAttachment)
{
	colorAttachments.add(colorAttachment);
}
RenderTargetLayout::RenderTargetLayout(Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachmet)
	: inputAttachments(), colorAttachments(colorAttachments), depthAttachment(depthAttachment)
{
}
RenderTargetLayout::RenderTargetLayout(Array<PRenderTargetAttachment> inputAttachments, Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment)
	: inputAttachments(inputAttachments), colorAttachments(colorAttachments), depthAttachment(depthAttachment)
{
}

Window::Window(const WindowCreateInfo &createInfo)
	: sizeX(createInfo.width), sizeY(createInfo.height), bFullscreen(createInfo.bFullscreen), title(createInfo.title), pixelFormat(createInfo.pixelFormat)
{
}

Window::~Window()
{
}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo &viewportInfo)
	: sizeX(viewportInfo.sizeX), sizeY(viewportInfo.sizeY), offsetX(viewportInfo.offsetX), offsetY(viewportInfo.offsetY), owner(owner)
{
}

Viewport::~Viewport()
{
}