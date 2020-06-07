#pragma once
#include "GraphicsResources.h"

namespace Seele
{
#define MAX_TEX_CHANNELS 4
enum class VertexAttribute
{
    POSITION,
    TEXCOORD,
    NORMAL,
    TANGENT,
    BITANGENT
};
struct MeshDescription
{
    Array<VertexAttribute> layout;
    Gfx::PVertexDeclaration declaration;
    uint32 getStride() const
    {
        return getNumFloats() * sizeof(float);
    }
    uint32 getNumFloats() const
    {
        uint32 vertexSize = 0;
        for(auto a : layout)
        {
            switch (a)
            {
            case VertexAttribute::POSITION:
            case VertexAttribute::NORMAL:
            case VertexAttribute::TANGENT:
            case VertexAttribute::BITANGENT:
                vertexSize += 3;
                break;
            case VertexAttribute::TEXCOORD:
                vertexSize += 2;
                break;
            }
        }
        return vertexSize;
    }
};
DECLARE_REF(MaterialAsset);
class Mesh
{
public:
    Mesh(MeshDescription description, Gfx::PIndexBuffer indexBuffer);
    ~Mesh();

    Gfx::PIndexBuffer indexBuffer;
    MeshDescription description;
    PMaterialAsset referencedMaterial;
};
DEFINE_REF(Mesh);
} // namespace Seele