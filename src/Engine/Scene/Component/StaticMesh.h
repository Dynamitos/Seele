#pragma once
#include "Graphics/GraphicsResources.h"

namespace Seele
{
namespace Component
{
struct StaticMesh
{
    PVertexShaderInput vertexBuffer;
    Gfx::PIndexBuffer indexBuffer;
    PMaterialAsset material;
};
} // namespace Component
} // namespace Seele
