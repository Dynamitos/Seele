#include "Resources.h"
#include "Graphics.h"
#include "Material/Material.h"
#include "Resources.h"


using namespace Seele;
using namespace Seele::Gfx;

QueueOwnedResource::QueueOwnedResource(QueueFamilyMapping mapping, QueueType startQueueType)
    : currentOwner(startQueueType), mapping(mapping) {}

QueueOwnedResource::~QueueOwnedResource() {}

void QueueOwnedResource::transferOwnership(QueueType newOwner) {
    if (mapping.needsTransfer(currentOwner, newOwner)) {
        executeOwnershipBarrier(newOwner);
    }
    currentOwner = newOwner;
}

void QueueOwnedResource::pipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                                         SePipelineStageFlags dstStage) {
    // maybe add some checks
    executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
