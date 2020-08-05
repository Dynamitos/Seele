#include "Mesh.h"

using namespace Seele;

Mesh::Mesh(MeshDescription description, Gfx::PIndexBuffer indexBuffer) 
    : description(description)
    , indexBuffer(indexBuffer)
{    
}

Mesh::~Mesh()
{
}
