#pragma once
#include "Containers/Array.h"
#include "Enums.h"
#include "Math/Math.h"

namespace Seele {
DECLARE_REF(Material)
namespace Gfx {
DECLARE_REF(DescriptorSet)
DECLARE_REF(Graphics)
DECLARE_REF(VertexBuffer)
DECLARE_REF(IndexBuffer)
DECLARE_REF(GraphicsPipeline)
DECLARE_REF(ComputePipeline)
class Sampler {
  public:
    virtual ~Sampler() {}
};
DEFINE_REF(Sampler)

struct QueueFamilyMapping {
    uint32 graphicsFamily;
    uint32 computeFamily;
    uint32 transferFamily;
    uint32 getQueueTypeFamilyIndex(Gfx::QueueType type) const {
        switch (type) {
        case Gfx::QueueType::GRAPHICS:
            return graphicsFamily;
        case Gfx::QueueType::COMPUTE:
            return computeFamily;
        case Gfx::QueueType::TRANSFER:
            return transferFamily;
        default:
            return 0x7fff;
        }
    }
    bool needsTransfer(Gfx::QueueType src, Gfx::QueueType dst) const {
        uint32 srcIndex = getQueueTypeFamilyIndex(src);
        uint32 dstIndex = getQueueTypeFamilyIndex(dst);
        return srcIndex != dstIndex;
    }
};

class QueueOwnedResource {
  public:
    QueueOwnedResource(QueueFamilyMapping mapping);
    virtual ~QueueOwnedResource();

    // Preliminary checks to see if the barrier should be executed at all
    void transferOwnership(QueueType newOwner);
    void pipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess, SePipelineStageFlags dstStage);

  protected:
    virtual void executeOwnershipBarrier(QueueType newOwner) = 0;
    virtual void executePipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                                        SePipelineStageFlags dstStage) = 0;
    QueueFamilyMapping mapping;
};
DEFINE_REF(QueueOwnedResource)

} // namespace Gfx
} // namespace Seele