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
    Gfx::VertexBuffer vertexBuffer;
    Gfx::IndexBuffer indexBuffer;
};
}