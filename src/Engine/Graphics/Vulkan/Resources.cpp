#include "Resources.h"
#include "Enums.h"
#include "Initializer.h"
#include "Graphics.h"

using namespace Seele;
using namespace Seele::Vulkan;

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
    : graphics(graphics)
    , signaled(false)
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
        return signaled;
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

DestructionManager::DestructionManager(PGraphics graphics)
    : graphics(graphics)
{
}

DestructionManager::~DestructionManager()
{
}

void DestructionManager::queueBuffer(PCmdBuffer cmd, VkBuffer buffer)
{
    buffers[cmd].add(buffer);
}

void DestructionManager::queueImage(PCmdBuffer cmd, VkImage image)
{
    images[cmd].add(image);
}

void DestructionManager::queueImageView(PCmdBuffer cmd, VkImageView image)
{
    views[cmd].add(image);
}

void DestructionManager::notifyCmdComplete(PCmdBuffer cmdbuffer)
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
    buffers[cmdbuffer].clear();
    images[cmdbuffer].clear();
    views[cmdbuffer].clear();
}

VertexDeclaration::VertexDeclaration(const Array<Gfx::VertexElement>& elementList) 
    : elementList(elementList)
{
}

VertexDeclaration::~VertexDeclaration() 
{   
}
