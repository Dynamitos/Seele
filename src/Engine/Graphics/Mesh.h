#pragma once
#include "GraphicsResources.h"
#include "Material/MaterialAsset.h"

namespace Seele
{
#define MAX_TEX_CHANNELS 8
struct MeshDescription
{
    Array<Gfx::VertexAttribute> layout;
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
            case Gfx::VertexAttribute::POSITION:
            case Gfx::VertexAttribute::NORMAL:
            case Gfx::VertexAttribute::TANGENT:
            case Gfx::VertexAttribute::BITANGENT:
                vertexSize += 3;
                break;
            case Gfx::VertexAttribute::TEXCOORD:
                vertexSize += 2;
                break;
            }
        }
        return vertexSize;
    }
    
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & layout;
        //TODO declaration
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
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & description;
        ar & referencedMaterial->getFullPath();
        //TODO: 
	}
};
DEFINE_REF(Mesh);
} // namespace Seele