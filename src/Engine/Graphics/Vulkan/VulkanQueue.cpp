#include "VulkanQueue.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"
#include "VulkanAllocator.h"
#include "VulkanCommandBuffer.h"
#include "VulkanGraphicsEnums.h"

using namespace Seele;
using namespace Seele::Vulkan;

Queue::Queue(PGraphics graphics, Gfx::QueueType queueType, uint32 familyIndex, uint32 queueIndex)
    : graphics(graphics)
    , familyIndex(familyIndex)
    , queueType(queueType)
{
    vkGetDeviceQueue(graphics->getDevice(), familyIndex, queueIndex, &queue);
}

Queue::~Queue()
{
}

void Queue::submitCommandBuffer(PCmdBuffer cmdBuffer, uint32 numSignalSemaphores, VkSemaphore *signalSemaphores)
{
    
    std::scoped_lock lck(queueLock);
    assert(cmdBuffer->state == CmdBuffer::State::Ended);

    PFence fence = cmdBuffer->fence;
    assert(!fence->isSignaled());

    const VkCommandBuffer cmdBuffers[] = {cmdBuffer->handle};

    VkSubmitInfo submitInfo =
        init::SubmitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = cmdBuffers;
    submitInfo.signalSemaphoreCount = static_cast<uint32>(numSignalSemaphores);
    submitInfo.pSignalSemaphores = signalSemaphores;

    Array<VkSemaphore> waitSemaphores;
    if (cmdBuffer->waitSemaphores.size() > 0)
    {
        for (PSemaphore semaphore : cmdBuffer->waitSemaphores)
        {
            waitSemaphores.add(semaphore->getHandle());
        }
        submitInfo.waitSemaphoreCount = static_cast<uint32>(cmdBuffer->waitSemaphores.size());
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = cmdBuffer->waitFlags.data();
    }
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence->getHandle()));
    cmdBuffer->state = CmdBuffer::State::Submitted;
    cmdBuffer->waitFlags.clear();
    cmdBuffer->waitSemaphores.clear();

    if (Gfx::waitIdleOnSubmit)
    {
        fence->wait(200 * 1000ull);
    }

    cmdBuffer->refreshFence();
    graphics->getStagingManager()->clearPending();
    
}