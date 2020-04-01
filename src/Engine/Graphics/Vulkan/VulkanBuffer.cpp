#include "VulkanGraphicsResources.h"
#include "VulkanInitializer.h"

using namespace Seele;
using namespace Seele::Vulkan;

QueueOwnedResource::QueueOwnedResource(PGraphics graphics, QueueType startQueueType)
	: graphics(graphics)
	, currentOwner(startQueueType)
{
}

QueueOwnedResource::~QueueOwnedResource()
{
}

Buffer::Buffer(PGraphics graphics, uint32 size, VkBufferUsageFlags usage, QueueType queueType)
	: QueueOwnedResource(graphics, queueType)
{
}

Buffer::~Buffer()
{
}

void Buffer::executeOwnershipBarrier(QueueType newOwner)
{
}
