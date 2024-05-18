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
    //graphics->getDestructionManager()->queueSemaphore(graphics->getGraphicsCommands()->getCommands(), handle);
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
    vkWaitForFences(graphics->getDevice(), 1, &fence, true, 100000);
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

void DestructionManager::queueResourceForDestruction(OCommandBoundResource resource)
{
    resources.add(std::move(resource));
}

void DestructionManager::notifyCommandComplete()
{
    for (size_t i = 0; i < resources.size(); ++i)
    {
        if(!resources[i]->isCurrentlyBound())
        {
            resources.removeAt(i, false);
            i--;
        }
    }
}

SamplerHandle::SamplerHandle(PGraphics graphics, VkSamplerCreateInfo createInfo)
    : CommandBoundResource(graphics)
{
    vkCreateSampler(graphics->getDevice(), &createInfo, nullptr, &sampler);
}

SamplerHandle::~SamplerHandle()
{
    vkDestroySampler(graphics->getDevice(), sampler, nullptr);
}

Sampler::Sampler(PGraphics graphics, VkSamplerCreateInfo createInfo)
    : graphics(graphics)
    , handle(new SamplerHandle(graphics, createInfo))
{}

Sampler::~Sampler()
{
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(handle));
}
