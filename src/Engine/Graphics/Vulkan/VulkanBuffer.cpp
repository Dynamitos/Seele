#include "VulkanGraphicsResources.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"
#include "VulkanCommandBuffer.h"
#include "VulkanAllocator.h"

using namespace Seele;
using namespace Seele::Vulkan;

struct PendingBuffer
{
	PStagingBuffer stagingBuffer;
	Gfx::QueueType prevQueue;
	bool bWriteOnly;
};

static Map<ShaderBuffer *, PendingBuffer> pendingBuffers;

ShaderBuffer::ShaderBuffer(PGraphics graphics, uint32 size, VkBufferUsageFlags usage, Gfx::QueueType queueType)
	: graphics(graphics)
	, currentBuffer(0)
	, size(size)
	, currentOwner(queueType)
{
	if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT ||
		usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ||
		usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)
	{
		numBuffers = 1;
	}
	else
	{
		numBuffers = Gfx::numFramesBuffered;
	}
	usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkBufferCreateInfo info =
		init::BufferCreateInfo(
			usage,
			size);
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uint32 queueFamilyIndex =  graphics->getFamilyMapping().getQueueTypeFamilyIndex(queueType);
	info.pQueueFamilyIndices = &queueFamilyIndex;
	info.queueFamilyIndexCount = 0;
	VkBufferMemoryRequirementsInfo2 bufferReqInfo;
	bufferReqInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
	bufferReqInfo.pNext = nullptr;
	VkMemoryDedicatedRequirements dedicatedRequirements;
	dedicatedRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
	dedicatedRequirements.pNext = nullptr;
	VkMemoryRequirements2 memRequirements;
	memRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
	memRequirements.pNext = &dedicatedRequirements;
	for (uint32 i = 0; i < numBuffers; ++i)
	{
		vkCreateBuffer(graphics->getDevice(), &info, nullptr, &buffers[i].buffer);
		bufferReqInfo.buffer = buffers[i].buffer;
		vkGetBufferMemoryRequirements2(graphics->getDevice(), &bufferReqInfo, &memRequirements);
		buffers[i].allocation = graphics->getAllocator()->allocate(memRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffers[i].buffer);
		vkBindBufferMemory(graphics->getDevice(), buffers[i].buffer, buffers[i].allocation->getHandle(), buffers[i].allocation->getOffset());
	}
}

ShaderBuffer::~ShaderBuffer()
{
	auto cmdBuffer = graphics->getQueueCommands(currentOwner)->getCommands();
	auto &deletionQueue = graphics->getDeletionQueue();
	VkDevice device = graphics->getDevice();
	VkBuffer buf[Gfx::numFramesBuffered];
	for (uint32 i = 0; i < numBuffers; ++i)
	{
		buf[i] = buffers[i].buffer;
		deletionQueue.addPendingDelete(cmdBuffer, [device, buf, i]() { vkDestroyBuffer(device, buf[i], nullptr); });
		buffers[i].allocation = nullptr;
	}
	graphics = nullptr;
}

VkDeviceSize ShaderBuffer::getOffset() const
{
	return buffers[currentBuffer].allocation->getOffset();
}

void ShaderBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
	VkBufferMemoryBarrier barrier =
		init::BufferMemoryBarrier();
	PCommandBufferManager sourceManager = graphics->getQueueCommands(currentOwner);
	PCommandBufferManager dstManager = nullptr;
	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	Gfx::QueueFamilyMapping mapping = graphics->getFamilyMapping();
	barrier.srcQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(currentOwner);
	barrier.dstQueueFamilyIndex = mapping.getQueueTypeFamilyIndex(newOwner);
	assert(barrier.srcQueueFamilyIndex != barrier.dstQueueFamilyIndex);
	if (currentOwner == Gfx::QueueType::TRANSFER || currentOwner == Gfx::QueueType::DEDICATED_TRANSFER)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (currentOwner == Gfx::QueueType::COMPUTE)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	else if (currentOwner == Gfx::QueueType::GRAPHICS)
	{
		barrier.srcAccessMask = getSourceAccessMask();
		srcStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	}
	if (newOwner == Gfx::QueueType::TRANSFER)
	{
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstManager = graphics->getTransferCommands();
	}
	else if (newOwner == Gfx::QueueType::DEDICATED_TRANSFER)
	{
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstManager = graphics->getDedicatedTransferCommands();
	}
	else if (newOwner == Gfx::QueueType::COMPUTE)
	{
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstManager = graphics->getComputeCommands();
	}
	else if (newOwner == Gfx::QueueType::GRAPHICS)
	{
		barrier.dstAccessMask = getDestAccessMask();
		dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		dstManager = graphics->getGraphicsCommands();
	}
	VkCommandBuffer srcCommand = sourceManager->getCommands()->getHandle();
	VkCommandBuffer dstCommand = dstManager->getCommands()->getHandle();
	VkBufferMemoryBarrier dynamicBarriers[Gfx::numFramesBuffered];
	for (uint32_t i = 0; i < numBuffers; ++i)
	{
		dynamicBarriers[i] = barrier;
		dynamicBarriers[i].buffer = buffers[i].buffer;
		dynamicBarriers[i].offset = 0;
		dynamicBarriers[i].size = size;
	}
	vkCmdPipelineBarrier(srcCommand, srcStage, dstStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
	vkCmdPipelineBarrier(dstCommand, srcStage, dstStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
	sourceManager->submitCommands();
	currentOwner = newOwner;
}

void *ShaderBuffer::lock(bool bWriteOnly)
{
	void *data = nullptr;

	/*if (bVolatile)
	{
		if (lockMode == RLM_ReadOnly)
		{
			assert(0);
		}
		else
		{
			throw new std::logic_error("TODO implement volatile buffers");
			//device->getRHIDevice()->getTemp
		}
	}*/
	//assert(bStatic || bDynamic || bUAV);

	PendingBuffer pending;
	pending.bWriteOnly = bWriteOnly;
	pending.prevQueue = currentOwner;
	if (bWriteOnly)
	{
		//requestOwnershipTransfer(Gfx::QueueType::DEDICATED_TRANSFER);
		PStagingBuffer stagingBuffer = graphics->getStagingManager()->allocateStagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		data = stagingBuffer->getMappedPointer();
		pending.stagingBuffer = stagingBuffer;
	}
	else
	{
		PCmdBuffer current = graphics->getQueueCommands(currentOwner)->getCommands();
		graphics->getQueueCommands(currentOwner)->submitCommands();
		graphics->getQueueCommands(currentOwner)->waitForCommands(current);

		requestOwnershipTransfer(Gfx::QueueType::DEDICATED_TRANSFER);
		VkCommandBuffer handle = graphics->getQueueCommands(currentOwner)->getCommands()->getHandle();

		VkBufferMemoryBarrier barrier =
			init::BufferMemoryBarrier();
		barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.buffer = buffers[currentBuffer].buffer;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.offset = 0;
		barrier.size = size;
		vkCmdPipelineBarrier(handle, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

		PStagingBuffer stagingBuffer = graphics->getStagingManager()->allocateStagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true);

		VkBufferCopy regions;
		regions.size = size;
		regions.srcOffset = 0;
		regions.dstOffset = 0;

		vkCmdCopyBuffer(handle, buffers[currentBuffer].buffer, stagingBuffer->getHandle(), 1, &regions);

		graphics->getQueueCommands(currentOwner)->submitCommands();
		vkQueueWaitIdle(graphics->getQueueCommands(currentOwner)->getQueue()->getHandle());

		stagingBuffer->flushMappedMemory();
		pending.stagingBuffer = stagingBuffer;

		data = stagingBuffer->getMappedPointer();
	}
	pendingBuffers[this] = pending;

	assert(data);
	return data;
}

void ShaderBuffer::unlock()
{
	auto found = pendingBuffers.find(this);
	if (found != pendingBuffers.end())
	{
		PendingBuffer pending = found->value;
		pending.stagingBuffer->flushMappedMemory();
		pendingBuffers.erase(this);
		if (pending.bWriteOnly)
		{
			PStagingBuffer stagingBuffer = pending.stagingBuffer;
			PCmdBuffer cmdBuffer = graphics->getQueueCommands(currentOwner)->getCommands();
			VkCommandBuffer cmdHandle = cmdBuffer->getHandle();

			VkBufferCopy region;
			std::memset(&region, 0, sizeof(VkBufferCopy));
			region.size = size;
			vkCmdCopyBuffer(cmdHandle, stagingBuffer->getHandle(), buffers[currentBuffer].buffer, 1, &region);
			graphics->getQueueCommands(currentOwner)->submitCommands();
		}
		//requestOwnershipTransfer(pending.prevQueue);
		graphics->getStagingManager()->releaseStagingBuffer(pending.stagingBuffer);
	}
}

UniformBuffer::UniformBuffer(PGraphics graphics, const UniformBufferCreateInfo &createInfo)
	: Gfx::UniformBuffer(graphics->getFamilyMapping(), createInfo.resourceData)
	, Vulkan::ShaderBuffer(graphics, createInfo.resourceData.size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, createInfo.resourceData.owner)
	, dedicatedStagingBuffer(nullptr)
{
	if(createInfo.bDynamic)
	{
		dedicatedStagingBuffer = graphics->getStagingManager()->allocateStagingBuffer(createInfo.resourceData.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	}
	if (createInfo.resourceData.data != nullptr)
	{
		void *data = lock();
		std::memcpy(data, createInfo.resourceData.data, createInfo.resourceData.size);
		unlock();
	}
}

UniformBuffer::~UniformBuffer()
{
}

bool UniformBuffer::updateContents(const BulkResourceData &resourceData) 
{
    if(!Gfx::UniformBuffer::updateContents(resourceData))
	{
		// no update was performed, skip
		return false;
	}
    void* data = lock();
    std::memcpy(data, resourceData.data, resourceData.size);
    unlock();
	return true;
}
void* UniformBuffer::lock(bool bWriteOnly)
{
	if(dedicatedStagingBuffer != nullptr)
	{
		return dedicatedStagingBuffer->getMappedPointer();
	}
	return ShaderBuffer::lock(bWriteOnly);
}

void UniformBuffer::unlock()
{
	if(dedicatedStagingBuffer != nullptr)
	{
		dedicatedStagingBuffer->flushMappedMemory();
		PCmdBuffer cmdBuffer = graphics->getQueueCommands(ShaderBuffer::currentOwner)->getCommands();
		VkCommandBuffer cmdHandle = cmdBuffer->getHandle();

		VkBufferCopy region;
		std::memset(&region, 0, sizeof(VkBufferCopy));
		region.size = ShaderBuffer::size;
		vkCmdCopyBuffer(cmdHandle, dedicatedStagingBuffer->getHandle(), buffers[currentBuffer].buffer, 1, &region);
		graphics->getQueueCommands(ShaderBuffer::currentOwner)->submitCommands();
	}
	else
	{
		ShaderBuffer::unlock();
	}
}

void UniformBuffer::requestOwnershipTransfer(Gfx::QueueType newOwner)
{
	Gfx::QueueOwnedResource::transferOwnership(newOwner);
	Vulkan::ShaderBuffer::currentOwner = newOwner;
}

void UniformBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
	Vulkan::ShaderBuffer::executeOwnershipBarrier(newOwner);
}

VkAccessFlags UniformBuffer::getSourceAccessMask()
{
	return VK_ACCESS_MEMORY_WRITE_BIT;
}

VkAccessFlags UniformBuffer::getDestAccessMask()
{
	return VK_ACCESS_UNIFORM_READ_BIT;
}

StructuredBuffer::StructuredBuffer(PGraphics graphics, const BulkResourceData &resourceData)
	: Gfx::StructuredBuffer(graphics->getFamilyMapping(), resourceData.owner)
	, Vulkan::ShaderBuffer(graphics, resourceData.size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, resourceData.owner)
{
	if (resourceData.data != nullptr)
	{
		void *data = lock();
		std::memcpy(data, resourceData.data, resourceData.size);
		unlock();
	}
}

StructuredBuffer::~StructuredBuffer()
{
}

void StructuredBuffer::requestOwnershipTransfer(Gfx::QueueType newOwner)
{
	Gfx::QueueOwnedResource::transferOwnership(newOwner);
}

void StructuredBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
	Vulkan::ShaderBuffer::executeOwnershipBarrier(newOwner);
}

VkAccessFlags StructuredBuffer::getSourceAccessMask()
{
	return VK_ACCESS_MEMORY_WRITE_BIT;
}

VkAccessFlags StructuredBuffer::getDestAccessMask()
{
	return VK_ACCESS_MEMORY_READ_BIT;
}

VertexBuffer::VertexBuffer(PGraphics graphics, const VertexBufferCreateInfo &resourceData)
	: Gfx::VertexBuffer(graphics->getFamilyMapping(), resourceData.numVertices, resourceData.vertexSize, resourceData.resourceData.owner)
	, Vulkan::ShaderBuffer(graphics, resourceData.resourceData.size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, resourceData.resourceData.owner)
{
	if (resourceData.resourceData.data != nullptr)
	{
		void *data = lock();
		std::memcpy(data, resourceData.resourceData.data, resourceData.resourceData.size);
		unlock();
	}
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::requestOwnershipTransfer(Gfx::QueueType newOwner)
{
	Gfx::QueueOwnedResource::transferOwnership(newOwner);
}

void VertexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
	Vulkan::ShaderBuffer::executeOwnershipBarrier(newOwner);
}

VkAccessFlags VertexBuffer::getSourceAccessMask()
{
	return VK_ACCESS_MEMORY_WRITE_BIT;
}

VkAccessFlags VertexBuffer::getDestAccessMask()
{
	return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
}

IndexBuffer::IndexBuffer(PGraphics graphics, const IndexBufferCreateInfo &resourceData)
	: Gfx::IndexBuffer(graphics->getFamilyMapping(), resourceData.resourceData.size, resourceData.indexType, resourceData.resourceData.owner)
	, Vulkan::ShaderBuffer(graphics, resourceData.resourceData.size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, resourceData.resourceData.owner)
{
	if (resourceData.resourceData.data != nullptr)
	{
		void *data = lock();
		std::memcpy(data, resourceData.resourceData.data, resourceData.resourceData.size);
		unlock();
	}
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::requestOwnershipTransfer(Gfx::QueueType newOwner)
{
	Gfx::QueueOwnedResource::transferOwnership(newOwner);
}

void IndexBuffer::executeOwnershipBarrier(Gfx::QueueType newOwner)
{
	Vulkan::ShaderBuffer::executeOwnershipBarrier(newOwner);
}

VkAccessFlags IndexBuffer::getSourceAccessMask()
{
	return VK_ACCESS_MEMORY_WRITE_BIT;
}

VkAccessFlags IndexBuffer::getDestAccessMask()
{
	return VK_ACCESS_INDEX_READ_BIT;
}