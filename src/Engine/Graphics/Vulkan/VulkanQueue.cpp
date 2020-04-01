#include "VulkanQueue.h"
#include "VulkanInitializer.h"
#include "VulkanGraphics.h"

using namespace Seele;
using namespace Seele::Vulkan;

Queue::Queue(PGraphics graphics, QueueType queueType, uint32 familyIndex, uint32 queueIndex)
    : familyIndex(familyIndex)
    , graphics(graphics)
    , queueType(queueType)
{   
    vkGetDeviceQueue(graphics->getDevice(), familyIndex, queueIndex, &queue);
}

Queue::~Queue()
{

}