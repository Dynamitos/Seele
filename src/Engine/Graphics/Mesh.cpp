#include "Mesh.h"
#include "VertexShaderInput.h"

using namespace Seele;

Mesh::Mesh(PVertexShaderInput vertexInput, Gfx::PIndexBuffer indexBuffer) 
    : indexBuffer(indexBuffer)
    , vertexInput(vertexInput)
{    
}

Mesh::~Mesh()
{
}
