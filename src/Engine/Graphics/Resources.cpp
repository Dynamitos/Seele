#include "Resources.h"

using namespace Seele;
using namespace Seele::Gfx;

QueueOwnedResource::QueueOwnedResource(QueueFamilyMapping mapping)
    : mapping(mapping) {}

QueueOwnedResource::~QueueOwnedResource() {}

void QueueOwnedResource::transferOwnership(QueueType newOwner) {
    executeOwnershipBarrier(newOwner);
}

void QueueOwnedResource::pipelineBarrier(SeAccessFlags srcAccess, SePipelineStageFlags srcStage, SeAccessFlags dstAccess,
                                         SePipelineStageFlags dstStage) {
    // maybe add some checks
    executePipelineBarrier(srcAccess, srcStage, dstAccess, dstStage);
}
