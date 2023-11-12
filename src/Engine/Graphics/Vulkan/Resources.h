#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include "Containers/List.h"
#include "Graphics/Resources.h"
#include "Allocator.h"

namespace Seele
{
namespace Vulkan
{

DECLARE_REF(DescriptorAllocator)
DECLARE_REF(CommandBufferManager)
DECLARE_REF(CmdBuffer)
DECLARE_REF(Graphics)
DECLARE_REF(SubAllocation)
class Semaphore
{
public:
    Semaphore(PGraphics graphics);
    virtual ~Semaphore();
    inline VkSemaphore getHandle() const
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
    inline VkFence getHandle() const
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
    void queueBuffer(PCmdBuffer cmd, VkBuffer buffer);
    void queueImage(PCmdBuffer cmd, VkImage image);
    void queueImageView(PCmdBuffer cmd, VkImageView view);
    void notifyCmdComplete(PCmdBuffer cmdbuffer);
private:
    PGraphics graphics;
    Map<PCmdBuffer, List<VkBuffer>> buffers;
    Map<PCmdBuffer, List<VkImage>> images;
    Map<PCmdBuffer, List<VkImageView>> views;
};
DEFINE_REF(DestructionManager)

class VertexDeclaration : public Gfx::VertexDeclaration
{
public:
    Array<Gfx::VertexElement> elementList;

    VertexDeclaration(const Array<Gfx::VertexElement>& elementList);
    virtual ~VertexDeclaration();
private:
};
DEFINE_REF(VertexDeclaration)

class SamplerState : public Gfx::SamplerState
{
public:
    VkSampler sampler;
};
DEFINE_REF(SamplerState)

} // namespace Vulkan
} // namespace Seele
