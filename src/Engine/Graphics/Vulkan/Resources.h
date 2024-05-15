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
DECLARE_REF(CommandBoundResource)
class DestructionManager
{
public:
    DestructionManager(PGraphics graphics);
    ~DestructionManager();
    void queueResourceForDestruction(OCommandBoundResource resource);
    void notifyCommandComplete();
private:
    PGraphics graphics;
    Array<OCommandBoundResource> resources;
};
DEFINE_REF(DestructionManager)

class CommandBoundResource
{
public:
    CommandBoundResource(PGraphics graphics)
        : graphics(graphics)
    {
    }
    virtual ~CommandBoundResource()
    {
        if (isCurrentlyBound())
            abort();
    }
    constexpr bool isCurrentlyBound() const { return bindCount > 0; }
    constexpr void bind() { bindCount++; }
    constexpr void unbind() { bindCount--; }
protected:
    PGraphics graphics;
    uint64 bindCount = 0;
};
DEFINE_REF(CommandBoundResource)

class Sampler : public Gfx::Sampler
{
public:
    VkSampler sampler;
};
DEFINE_REF(Sampler)

} // namespace Vulkan
} // namespace Seele
