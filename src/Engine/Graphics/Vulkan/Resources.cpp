#include "Resources.h"
#include "Enums.h"
#include "Graphics.h"
#include "Command.h"
#include "Window.h"

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
    graphics->getDestructionManager()->queueSemaphore(graphics->getGraphicsCommands()->getCommands(), handle);
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
    VkResult r = vkGetFenceStatus(graphics->getDevice(), fence);
    if (r == VK_SUCCESS)
    {
        signaled = true;
        return true;
    }
    if (r == VK_NOT_READY)
    {
        return false;
    }
    else
    {
        VK_CHECK(r);
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
    VkResult r = vkWaitForFences(graphics->getDevice(), 1, fences, true, timeout);
    if (r == VK_SUCCESS)
    {
        signaled = true;
    }
    else if (r == VK_TIMEOUT)
    {
        return;
    }
    else if (r != VK_NOT_READY)
    {
        VK_CHECK(r);
    }
}

DestructionManager::DestructionManager(PGraphics graphics)
    : graphics(graphics)
{
}

DestructionManager::~DestructionManager()
{
}

void DestructionManager::queueBuffer(PCommand cmd, VkBuffer buffer, VmaAllocation alloc)
{
    buffers[cmd].add({buffer, alloc});
}

void DestructionManager::queueImage(PCommand cmd, VkImage image, VmaAllocation alloc)
{
    images[cmd].add({image, alloc});
}

void DestructionManager::queueImageView(PCommand cmd, VkImageView image)
{
    views[cmd].add(image);
}

void DestructionManager::queueSemaphore(PCommand cmd, VkSemaphore sem)
{
    sems[cmd].add(sem);
}

void DestructionManager::queueRenderPass(PCommand cmd, VkRenderPass renderPass)
{
    renderPasses[cmd].add(renderPass);
}

void DestructionManager::queueDescriptorPool(PCommand cmd, VkDescriptorPool pool)
{
    pools[cmd].add(pool);
}

void DestructionManager::notifyCmdComplete(PCommand cmd)
{
    for(auto [buf, alloc] : buffers[cmd])
    {
        vmaDestroyBuffer(graphics->getAllocator(), buf, alloc);
    }
    for(auto view : views[cmd])
    {
        vkDestroyImageView(graphics->getDevice(), view, nullptr);
    }
    for(auto [img, alloc] : images[cmd])
    {
        vmaDestroyImage(graphics->getAllocator(), img, alloc);
    }
    for (auto sem : sems[cmd])
    {
        vkDestroySemaphore(graphics->getDevice(), sem, nullptr);
    }
    for (auto pool : pools[cmd])
    {
        vkDestroyDescriptorPool(graphics->getDevice(), pool, nullptr);
    }
    for (auto renderPass : renderPasses[cmd])
    {
        vkDestroyRenderPass(graphics->getDevice(), renderPass, nullptr);
    }
    buffers[cmd].clear();
    images[cmd].clear();
    views[cmd].clear();
    sems[cmd].clear();
    pools[cmd].clear();
    renderPasses[cmd].clear();
}
