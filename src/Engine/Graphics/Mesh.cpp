#include "Mesh.h"
#include "VertexShaderInput.h"

using namespace Seele;

Mesh::Mesh(PVertexShaderInput vertexInput, Gfx::PIndexBuffer indexBuffer) 
    : vertexInput(vertexInput)
    , indexBuffer(indexBuffer)
{    
}

Mesh::~Mesh()
{
}
