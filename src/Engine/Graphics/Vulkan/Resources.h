#pragma once
#include "Containers/List.h"
#include "Graphics/Resources.h"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Seele {
namespace Vulkan {
DECLARE_REF(DescriptorPool)
DECLARE_REF(CommandPool)
DECLARE_REF(Command)
DECLARE_REF(Graphics)

class CommandBoundResource {
  public:
    CommandBoundResource(PGraphics graphics, const std::string& name) : graphics(graphics), name(name) {}
    virtual ~CommandBoundResource() {
        if (isCurrentlyBound())
            abort();
    }
    constexpr bool isCurrentlyBound() const { return bindCount > 0; }
    constexpr void bind() { bindCount++; }
    constexpr void unbind() { bindCount--; }

  protected:
    PGraphics graphics;
    std::string name;
    uint64 bindCount = 0;
};
DEFINE_REF(CommandBoundResource)

class SemaphoreHandle : public CommandBoundResource {
  public:
    SemaphoreHandle(PGraphics graphics, const std::string& name);
    virtual ~SemaphoreHandle();

    constexpr VkSemaphore getHandle() const { return handle; }

  private:
    VkSemaphore handle;
};
DEFINE_REF(SemaphoreHandle)

class Semaphore {
  public:
    Semaphore(PGraphics graphics);
    virtual ~Semaphore();
    // call when you need a new semaphore
    void rotateSemaphore();
    // call when the semaphore is to signal something, for example after using it in vkAcquireImage
    void encodeSignal() {
        if (handles.size() == 0)
            return;
        handles[currentHandle]->bind();
    }
    // call when the semaphore has been signalled
    void resolveSignal() {
        if (handles.size() == 0)
            return;
        handles[currentHandle]->unbind();
    }
    constexpr VkSemaphore getHandle() const { return handles[currentHandle]->getHandle(); }
    PSemaphoreHandle getCurrentSemaphore() const { return handles[currentHandle]; }

  private:
    Array<OSemaphoreHandle> handles;
    uint32 currentHandle = 0;
    PGraphics graphics;
};
DEFINE_REF(Semaphore)

class Fence {
  public:
    Fence(PGraphics graphics);
    ~Fence();
    void submit();
    bool isSignaled();
    void reset();
    constexpr VkFence getHandle() const { return fence; }
    void wait(uint64 timeout);
    bool operator<(const Fence& other) const { return fence < other.fence; }

  private:
    PGraphics graphics;
    enum class Status {
        Ready,
        InUse,
        Signalled,
    };
    Status status;
    VkFence fence;
};
DEFINE_REF(Fence)
class DestructionManager {
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

class SamplerHandle : public CommandBoundResource {
  public:
    SamplerHandle(PGraphics graphics, VkSamplerCreateInfo createInfo);
    virtual ~SamplerHandle();
    VkSampler sampler;
};
DEFINE_REF(SamplerHandle)

class Sampler : public Gfx::Sampler {
  public:
    Sampler(PGraphics graphics, VkSamplerCreateInfo createInfo);
    virtual ~Sampler();
    PSamplerHandle getHandle() const { return handle; }
    VkSampler getSampler() const { return handle->sampler; }

  private:
    PGraphics graphics;
    OSamplerHandle handle;
};
DEFINE_REF(Sampler)

} // namespace Vulkan
} // namespace Seele
