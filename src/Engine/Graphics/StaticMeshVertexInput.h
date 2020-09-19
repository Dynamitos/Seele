#pragma once
#include "VertexShaderInput.h"

namespace Seele
{
enum { MAX_TEXCOORDS = 4 };
struct StaticMeshDataType
{
    VertexStreamComponent positionStream;
    VertexStreamComponent tangentBasisComponents[2];
    
    //Dont forget these are 4 component vectors
    Array<VertexStreamComponent> textureCoordinates;

    VertexStreamComponent colorComponent;
};
class StaticMeshVertexInput : public VertexShaderInput
{
    DECLARE_VERTEX_INPUT_TYPE(StaticMeshVertexInput);
public:
    StaticMeshVertexInput(std::string name);
    virtual ~StaticMeshVertexInput();
    void setPositionStream(const VertexStreamComponent& positionStream);
    void setTangentXStream(const VertexStreamComponent& tangentXStream);
    void setTangentZStream(const VertexStreamComponent& tangentZStream);
    void setTexCoordStream(uint32 index, const VertexStreamComponent& textureStream);
    void setColorStream(const VertexStreamComponent& colorStream);
private:
    StaticMeshDataType data;
};
DEFINE_REF(StaticMeshVertexInput);
}