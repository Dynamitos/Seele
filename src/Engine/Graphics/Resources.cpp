#include "GraphicsResources.h"
#include "Graphics.h"
#include "RenderPass/DepthPrepass.h"
#include "RenderPass/BasePass.h"
#include "Material/Material.h"

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

