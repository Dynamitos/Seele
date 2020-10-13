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
    virtual void init(Gfx::PGraphics graphics) override;
    void setData(StaticMeshDataType&& data);
private:
    StaticMeshDataType data;
};
DEFINE_REF(StaticMeshVertexInput);
}