#pragma once
#include "Resources.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Command)
DECLARE_REF(Graphics)
class Queue
{
public:
    Queue(PGraphics graphics, Gfx::QueueType queueType, uint32 familyIndex, uint32 queueIndex);
    virtual ~Queue();
    void submitCommandBuffer(PCommand command, uint32 numSignalSemaphores = 0, VkSemaphore *signalSemaphore = nullptr);
    void submitCommandBuffer(PCommand command, VkSemaphore signalSemaphore)
    {
        submitCommandBuffer(command, 1, &signalSemaphore);
    }
    constexpr uint32 getFamilyIndex() const
    {
        return familyIndex;
    }
    constexpr VkQueue getHandle() const
    {
        return queue;
    }

private:
    std::mutex queueLock;
    PGraphics graphics;
    VkQueue queue;
    uint32 familyIndex;
    Gfx::QueueType queueType;
};
DEFINE_REF(Queue)
} // namespace Vulkan
} // namespace Seele