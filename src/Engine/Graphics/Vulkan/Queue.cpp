#include "Queue.h"
#include "Graphics.h"
#include "Allocator.h"
#include "Command.h"

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

void Queue::submitCommandBuffer(PCommand command, uint32 numSignalSemaphores, VkSemaphore *signalSemaphores)
{
    std::unique_lock lock(queueLock);
    assert(command->state == Command::State::End);

    PFence fence = command->fence;
    assert(!fence->isSignaled());

    VkCommandBuffer cmdHandle = command->handle;

    Array<VkSemaphore> waitSemaphores;
    if (command->waitSemaphores.size() > 0)
    {
        for (PSemaphore semaphore : command->waitSemaphores)
        {
            waitSemaphores.add(semaphore->getHandle());
        }
    }

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = static_cast<uint32>(command->waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = command->waitFlags.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdHandle,
        .signalSemaphoreCount = static_cast<uint32>(numSignalSemaphores),
        .pSignalSemaphores = signalSemaphores,
    };
    
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence->getHandle()));
    command->state = Command::State::Submit;
    command->waitFlags.clear();
    command->waitSemaphores.clear();

    if (Gfx::waitIdleOnSubmit)
    {
        fence->wait(200 * 1000ull);
    }

    command->checkFence();    
}