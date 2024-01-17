#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "Containers/List.h"
#include "Graphics/Resources.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(DescriptorPool)
DECLARE_REF(CommandPool)
DECLARE_REF(Command)
DECLARE_REF(Graphics)
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
    void queueBuffer(PCommand cmd, VkBuffer buffer, VmaAllocation alloc);
    void queueImage(PCommand cmd, VkImage image, VmaAllocation alloc);
    void queueImageView(PCommand cmd, VkImageView view);
    void queueSemaphore(PCommand cmd, VkSemaphore sem);
    void queueRenderPass(PCommand cmd, VkRenderPass renderPass);
    void queueDescriptorPool(PCommand cmd, VkDescriptorPool pool);
    void notifyCmdComplete(PCommand cmdbuffer);
private:
    PGraphics graphics;
    Map<PCommand, List<Pair<VkBuffer, VmaAllocation>>> buffers;
    Map<PCommand, List<Pair<VkImage, VmaAllocation>>> images;
    Map<PCommand, List<VkImageView>> views;
    Map<PCommand, List<VkSemaphore>> sems;
    Map<PCommand, List<VkRenderPass>> renderPasses;
    Map<PCommand, List<VkDescriptorPool>> pools;
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
