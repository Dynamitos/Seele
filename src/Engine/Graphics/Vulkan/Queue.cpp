#include "Queue.h"
#include "Graphics.h"
#include "Allocator.h"
#include "Command.h"
#include "Enums.h"

using namespace Seele;
using namespace Seele::Vulkan;

static constexpr bool waitIdleOnSubmit = false;

Queue::Queue(PGraphics graphics, uint32 familyIndex, uint32 queueIndex)
    : graphics(graphics)
    , familyIndex(familyIndex)
{
    vkGetDeviceQueue(graphics->getDevice(), familyIndex, queueIndex, &queue);
}

Queue::~Queue()
{
}

void Queue::submitCommandBuffer(PCommand command, const Array<VkSemaphore>& signalSemaphores)
{
    std::unique_lock lock(queueLock);
    assert(command->state == Command::State::End);

    assert(!(command->fence->isSignaled()));

    VkCommandBuffer cmdHandle = command->handle;

    Array<VkSemaphore> waitSemaphores;
    for (PSemaphore semaphore : command->waitSemaphores)
    {
        waitSemaphores.add(semaphore->getHandle());
    }
    
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = static_cast<uint32>(command->waitSemaphores.size()),
        .pWaitSemaphores = waitSemaphores.data(),
        .pWaitDstStageMask = command->waitFlags.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdHandle,
        .signalSemaphoreCount = static_cast<uint32>(signalSemaphores.size()),
        .pSignalSemaphores = signalSemaphores.data(),
    };
    
    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, command->fence->getHandle()));
    command->fence->submit();
    command->state = Command::State::Submit;
    command->waitFlags.clear();
    command->waitSemaphores.clear();

    if (waitIdleOnSubmit)
    {
        command->fence->wait(1000 * 1000ull);
    }

    command->checkFence();
}