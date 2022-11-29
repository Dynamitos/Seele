#include "Collider.h"

using namespace Seele;
using namespace Seele::Component;

Collider Collider::transform(const Transform& transform) const
{
    return Collider{
        .type = this->type,
        .boundingbox = this->boundingbox.getTransformedBox(transform.toMatrix()),
        .physicsMesh = this->physicsMesh.transform(transform),
    };
}
