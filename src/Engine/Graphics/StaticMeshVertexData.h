#include "VertexData.h"

namespace Seele
{
class StaticMeshVertexData : public VertexData
{
public:
    StaticMeshVertexData();
    virtual ~StaticMeshVertexData();
private:
    Gfx::PShaderBuffer texCoords;
    Gfx::PShaderBuffer normals;
};
}