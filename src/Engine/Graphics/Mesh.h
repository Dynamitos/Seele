#pragma once
#include "GraphicsResources.h"

namespace Seele
{
class Mesh
{
public:
    Mesh(Gfx::PVertexBuffer vertexBuffer, Gfx::PIndexBuffer indexBuffer);
    ~Mesh();

private:
    Gfx::PVertexBuffer vertexBuffer;
    Gfx::PIndexBuffer indexBuffer;
};
DEFINE_REF(Mesh);
} // namespace Seele