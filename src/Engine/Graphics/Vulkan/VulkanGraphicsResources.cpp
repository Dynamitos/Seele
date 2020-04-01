#include "VulkanGraphicsResources.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"
#include "VulkanGraphicsEnums.h"

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
    vkDestroySemaphore(graphics->getDevice(), handle, nullptr);
}