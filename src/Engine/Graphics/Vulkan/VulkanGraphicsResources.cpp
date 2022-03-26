#include "VulkanGraphicsResources.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanCommandBuffer.h"

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
    , signaled("Fence")
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
        signaled.raise();
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
        signaled.reset();
    }
}

void Fence::wait(uint32 timeout)
{
    VkFence fences[] = {fence};
    VkResult r = vkWaitForFences(graphics->getDevice(), 1, fences, true, timeout * 1000ull);
    switch (r)
    {
    case VK_SUCCESS:
        signaled.raise();
        break;
    case VK_TIMEOUT:
        break;
    default:
        VK_CHECK(r);
        break;
    }
}

VertexDeclaration::VertexDeclaration(const Array<Gfx::VertexElement>& elementList) 
    : elementList(elementList)
{
}

VertexDeclaration::~VertexDeclaration() 
{   
}
