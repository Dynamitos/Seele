#include "Resources.h"
#include "Enums.h"
#include "Initializer.h"
#include "Graphics.h"

using namespace Seele;
using namespace Seele::Vulkan;

Semaphore::Semaphore(PGraphics graphics)
    : graphics(graphics)
{
    VkSemaphoreCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    VK_CHECK(vkCreateSemaphore(graphics->getDevice(), &info, nullptr, &handle));
}

Semaphore::~Semaphore()
{
    uint64 value = 0;
    VkSemaphoreWaitInfo waitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .pNext = nullptr,
        .flags = 0,
        .semaphoreCount = 1,
        .pSemaphores = &handle,
        .pValues = &value,
    };
    VK_CHECK(vkWaitSemaphores(graphics->getDevice(), &waitInfo, 10000));
    vkDestroySemaphore(graphics->getDevice(), handle, nullptr);
}

Fence::Fence(PGraphics graphics)
    : graphics(graphics)
    , signaled(false)
{
    VkFenceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };
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
    if (res == VK_SUCCESS)
    {
        signaled = true;
        return true;
    }
    if (res == VK_NOT_READY)
    {
        return false;
    }
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
    VkResult res = vkWaitForFences(graphics->getDevice(), 1, fences, true, timeout);
    if (res == VK_SUCCESS)
    {
        signaled = true;
    }
    if (res != VK_NOT_READY)
    {
        VK_CHECK(res);
    }
}

DestructionManager::DestructionManager(PGraphics graphics)
    : graphics(graphics)
{
}

DestructionManager::~DestructionManager()
{
}

void DestructionManager::queueBuffer(PCommand cmd, VkBuffer buffer)
{
    buffers[cmd].add(buffer);
}

void DestructionManager::queueImage(PCommand cmd, VkImage image)
{
    images[cmd].add(image);
}

void DestructionManager::queueImageView(PCommand cmd, VkImageView image)
{
    views[cmd].add(image);
}

void DestructionManager::queueSemaphore(PCommand cmd, VkSemaphore sem)
{
    sems[cmd].add(sem);
}

void DestructionManager::notifyCmdComplete(PCommand cmdbuffer)
{
    for(auto buf : buffers[cmdbuffer])
    {
        vkDestroyBuffer(graphics->getDevice(), buf, nullptr);
    }
    for(auto view : views[cmdbuffer])
    {
        vkDestroyImageView(graphics->getDevice(), view, nullptr);
    }
    for(auto img : images[cmdbuffer])
    {
        vkDestroyImage(graphics->getDevice(), img, nullptr);
    }
    for (auto sem : sems[cmdbuffer])
    {
        vkDestroySemaphore(graphics->getDevice(), sem, nullptr);
    }
    buffers[cmdbuffer].clear();
    images[cmdbuffer].clear();
    views[cmdbuffer].clear();
    sems[cmdbuffer].clear();
}
