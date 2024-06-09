#pragma once
#include "Resources.h"

namespace Seele {
namespace Vulkan {
DECLARE_REF(Command)
DECLARE_REF(Graphics)
class Queue {
  public:
    Queue(PGraphics graphics, uint32 familyIndex, uint32 queueIndex);
    virtual ~Queue();
    void submitCommandBuffer(PCommand command, const Array<VkSemaphore>& signalSemaphore);
    constexpr uint32 getFamilyIndex() const { return familyIndex; }
    constexpr VkQueue getHandle() const { return queue; }

  private:
    std::mutex queueLock;
    PGraphics graphics;
    VkQueue queue;
    uint32 familyIndex;
};
DEFINE_REF(Queue)
} // namespace Vulkan
} // namespace Seele