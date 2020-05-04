#include "VulkanGraphicsResources.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanCommandBuffer.h"

using namespace Seele;
using namespace Seele::Vulkan;

List<QueueOwnedResourceDeletion::PendingItem> QueueOwnedResourceDeletion::deletionQueue;
volatile bool QueueOwnedResourceDeletion::running;
std::mutex QueueOwnedResourceDeletion::mutex;
std::condition_variable QueueOwnedResourceDeletion::cv;

QueueOwnedResourceDeletion::QueueOwnedResourceDeletion()
{
    worker = std::thread(&run);
    running = true;
}

QueueOwnedResourceDeletion::~QueueOwnedResourceDeletion()
{
    worker.join();
}

void QueueOwnedResourceDeletion::addPendingDelete(PFence fence, std::function<void()> func)
{
    PendingItem item;
    item.fence = fence;
    item.func = func;
    deletionQueue.add(item);
    std::unique_lock<std::mutex> lock(mutex);
    cv.notify_all();
}

void QueueOwnedResourceDeletion::run()
{
    while (running || !deletionQueue.empty())
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock);
        auto entry = deletionQueue.begin();
        PFence fence = entry->fence;
        fence->wait(1000ull);
        if (fence->isSignaled())
        {
            entry->func();
            deletionQueue.remove(entry);
        }
    }
}

QueueOwnedResource::QueueOwnedResource(PGraphics graphics, Gfx::QueueType startQueueType)
    : graphics(graphics), currentOwner(startQueueType)
{
}

QueueOwnedResource::~QueueOwnedResource()
{
    cachedCmdBufferManager = nullptr;
    graphics = nullptr;
}

void QueueOwnedResource::transferOwnership(Gfx::QueueType newOwner)
{
    if (graphics->getFamilyMapping().needsTransfer(currentOwner, newOwner))
    {
        executeOwnershipBarrier(newOwner);
        currentOwner = newOwner;
    }
}

PCommandBufferManager QueueOwnedResource::getCommands()
{
    if (cachedCmdBufferManager != nullptr)
    {
        assert(cachedCmdBufferManager->getQueue()->getFamilyIndex() == graphics->getFamilyMapping().getQueueTypeFamilyIndex(currentOwner));
        return cachedCmdBufferManager;
    }
    switch (currentOwner)
    {
    case Gfx::QueueType::GRAPHICS:
        return graphics->getGraphicsCommands();
    case Gfx::QueueType::COMPUTE:
        return graphics->getComputeCommands();
    case Gfx::QueueType::TRANSFER:
        return graphics->getTransferCommands();
    case Gfx::QueueType::DEDICATED_TRANSFER:
        return graphics->getDedicatedTransferCommands();
    }
    return nullptr;
}

Semaphore::Semaphore(PGraphics graphics)
    : graphics(graphics)
{
    VkSemaphoreCreateInfo info =
        init::SemaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(graphics->getDevice(), &info, nullptr, &handle));
}

Semaphore::~Semaphore()
{
    graphics = nullptr;
    vkDestroySemaphore(graphics->getDevice(), handle, nullptr);
}

Fence::Fence(PGraphics graphics)
    : graphics(graphics), signaled(false)
{
    VkFenceCreateInfo info =
        init::FenceCreateInfo(0);
    VK_CHECK(vkCreateFence(graphics->getDevice(), &info, nullptr, &fence));
}

Fence::~Fence()
{
    vkDestroyFence(graphics->getDevice(), fence, nullptr);
}

bool Fence::isSignaled()
{
    if (signaled)
    {
        return true;
    }
    VkResult res = vkGetFenceStatus(graphics->getDevice(), fence);
    switch (res)
    {
    case VK_SUCCESS:
        signaled = true;
        return true;
    case VK_NOT_READY:
        break;
    default:
        break;
    }
    return false;
}

void Fence::reset()
{
    if (signaled)
    {
        vkResetFences(graphics->getDevice(), 1, &fence);
        signaled = false;
    }
}

void Fence::wait(uint32 timeout)
{
    VkFence fences[] = {fence};
    VkResult r = vkWaitForFences(graphics->getDevice(), 1, fences, true, timeout * 1000ull);
    switch (r)
    {
    case VK_SUCCESS:
        signaled = true;
        break;
    case VK_TIMEOUT:
        break;
    default:
        VK_CHECK(r);
        break;
    }
}
