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

void DestructionManager::queueRenderPass(PCommand cmd, VkRenderPass renderPass)
{
    renderPasses[cmd].add(renderPass);
}

void DestructionManager::queueDescriptorPool(PCommand cmd, VkDescriptorPool pool)
{
    pools[cmd].add(pool);
}

void DestructionManager::queueDescriptorSet(PCommand cmd, Pair<VkDescriptorSet, VkDescriptorPool> set)
{
    sets[cmd].add(set);
}

void DestructionManager::queueAllocation(PCommand cmd, OSubAllocation alloc)
{
    allocs[cmd].add(std::move(alloc));
}

void DestructionManager::notifyCmdComplete(PCommand cmd)
{
    for(auto buf : buffers[cmd])
    {
        vkDestroyBuffer(graphics->getDevice(), buf, nullptr);
    }
    for(auto view : views[cmd])
    {
        vkDestroyImageView(graphics->getDevice(), view, nullptr);
    }
    for(auto img : images[cmd])
    {
        vkDestroyImage(graphics->getDevice(), img, nullptr);
    }
    for (auto sem : sems[cmd])
    {
        vkDestroySemaphore(graphics->getDevice(), sem, nullptr);
    }
    for (auto [set, pool] : sets[cmd])
    {
        vkFreeDescriptorSets(graphics->getDevice(), pool, 1, &set);
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
    renderPasses[cmd].clear();
    allocs[cmd].clear();
}
