#pragma once
#include "Graphics/GraphicsResources.h"

namespace Seele
{
struct PrimitiveUniformBuffer
{
    Matrix4 localToWorld;
    Matrix4 worldToLocal;
    Vector4 actorWorldPosition;
};
} // namespace Seele
