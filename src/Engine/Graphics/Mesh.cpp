#include "Mesh.h"

using namespace Seele;

Mesh::Mesh(Gfx::PVertexBuffer vertexBuffer, Gfx::PIndexBuffer indexBuffer) 
    : vertexBuffer(vertexBuffer)
    , indexBuffer(indexBuffer)
{    
}

Mesh::~Mesh()
{
}