#pragma once
#include "GraphicsResources.h"
#include "Material/MaterialAsset.h"

namespace Seele
{
DECLARE_REF(MaterialAsset)
class Mesh
{
public:
    Mesh(PVertexShaderInput vertexInput, Gfx::PIndexBuffer indexBuffer);
    ~Mesh();

    Gfx::PIndexBuffer indexBuffer;
    PVertexShaderInput vertexInput;
    PMaterialAsset referencedMaterial;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive&, const unsigned int)
	{
    }
};
DEFINE_REF(Mesh)
} // namespace Seele