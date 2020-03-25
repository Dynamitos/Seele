#include "VulkanGraphicsResources.h"
#include "VulkanInitializer.h"

using namespace Seele;
using namespace Seele::Vulkan;

QueueOwnedResource::QueueOwnedResource(WGraphics graphics, QueueType startQueueType)
	: graphics(graphics)
	, currentOwner(startQueueType)
{
}

QueueOwnedResource::~QueueOwnedResource()
{
}

Buffer::Buffer(WGraphics graphics, uint32 size, VkBufferUsageFlags usage, QueueType queueType)
	: QueueOwnedResource(graphics, queueType)
{
}

Buffer::~Buffer()
{
}

void Buffer::executeOwnershipBarrier(QueueType newOwner)
{
}
