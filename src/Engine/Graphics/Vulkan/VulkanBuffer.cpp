#include "VulkanGraphicsResources.h"

using namespace Seele::Vulkan;

void Buffer::executeOwnershipBarrier(QueueType newOwner)
{
	VkBufferMemoryBarrier barrier =
		init::BufferMemoryBarrier();
	CommandBufferManager* sourceManager = getCommands();
	CommandBufferManager* dstManager = nullptr;
	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	if (currentOwner == QueueType::TRANSFER)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.srcQueueFamilyIndex = device->getTransferQueue()->getFamilyIndex();
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (currentOwner == QueueType::COMPUTE)
	{
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.srcQueueFamilyIndex = device->getComputeQueue()->getFamilyIndex();
		srcStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	else if (currentOwner == QueueType::GRAPHICS)
	{
		barrier.srcAccessMask = getSourceAccessMask();
		barrier.srcQueueFamilyIndex = device->getGraphicsQueue()->getFamilyIndex();
		srcStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	}
	if (newOwner == QueueType::TRANSFER)
	{
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstQueueFamilyIndex = device->getTransferQueue()->getFamilyIndex();
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstManager = device->getRHIDevice().getTransferCommands();
	}
	else if (newOwner == QueueType::COMPUTE)
	{
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstQueueFamilyIndex = device->getComputeQueue()->getFamilyIndex();
		dstStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		dstManager = device->getRHIDevice().getComputeCommands();
	}
	else if (newOwner == QueueType::GRAPHICS)
	{
		barrier.dstAccessMask = getDestAccessMask();
		barrier.dstQueueFamilyIndex = device->getGraphicsQueue()->getFamilyIndex();
		dstStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		dstManager = device->getRHIDevice().getGraphicsCommands();
	}
	VkCommandBuffer srcCommand = sourceManager->getCmdBuffer()->getHandle();
	VkCommandBuffer dstCommand = dstManager->getCmdBuffer()->getHandle();
	const bool bDynamic = (rhiUsage & BUF_Dynamic) != 0;
	uint32_t numBuffers = bDynamic ? NUM_BUFFERS : 1;
	VkBufferMemoryBarrier dynamicBarriers[NUM_BUFFERS];
	for (uint32_t i = 0; i < numBuffers; ++i)
	{
		dynamicBarriers[i] = barrier;
		dynamicBarriers[i].buffer = buffers[i]->getHandle();
		dynamicBarriers[i].offset = buffers[i]->getOffset();
		dynamicBarriers[i].size = buffers[i]->getSize();
	}
	vkCmdPipelineBarrier(srcCommand, srcStage, dstStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
	vkCmdPipelineBarrier(dstCommand, srcStage, dstStage, 0, 0, nullptr, numBuffers, dynamicBarriers, 0, nullptr);
	sourceManager->submitCommands();
}