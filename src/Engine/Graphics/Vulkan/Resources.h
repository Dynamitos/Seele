#pragma once
#include <vulkan/vulkan.h>
#include "Containers/List.h"
#include "Graphics/Resources.h"
#include "Allocator.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(DescriptorAllocator)
DECLARE_REF(CommandPool)
DECLARE_REF(Command)
DECLARE_REF(Graphics)
DECLARE_REF(SubAllocation)
class Semaphore
{
public:
    Semaphore(PGraphics graphics);
    virtual ~Semaphore();
    constexpr VkSemaphore getHandle() const
    {
        return handle;
    }
private:
    VkSemaphore handle;
    PGraphics graphics;
};
DEFINE_REF(Semaphore)

class Fence
{
public:
    Fence(PGraphics graphics);
    ~Fence();
    bool isSignaled();
    void reset();
    constexpr VkFence getHandle() const
    {
        return fence;
    }
    void wait(uint32 timeout);
    bool operator<(const Fence &other) const
    {
        return fence < other.fence;
    }

private:
    PGraphics graphics;
    bool signaled;
    VkFence fence;
};
DEFINE_REF(Fence)

class DestructionManager
{
public:
    DestructionManager(PGraphics graphics);
    ~DestructionManager();
    void queueBuffer(PCommand cmd, VkBuffer buffer);
    void queueImage(PCommand cmd, VkImage image);
    void queueImageView(PCommand cmd, VkImageView view);
    void queueSemaphore(PCommand cmd, VkSemaphore sem);
    void notifyCmdComplete(PCommand cmdbuffer);
private:
    PGraphics graphics;
    Map<PCommand, List<VkBuffer>> buffers;
    Map<PCommand, List<VkImage>> images;
    Map<PCommand, List<VkImageView>> views;
    Map<PCommand, List<VkSemaphore>> sems;
};
DEFINE_REF(DestructionManager)

class Sampler : public Gfx::Sampler
{
public:
    VkSampler sampler;
};
DEFINE_REF(Sampler)

} // namespace Vulkan
} // namespace Seele
