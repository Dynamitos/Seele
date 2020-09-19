#include "StaticMeshVertexInput.h"

using namespace Seele;

StaticMeshVertexInput::StaticMeshVertexInput(std::string name) 
    : VertexShaderInput(name)
{
    
}

StaticMeshVertexInput::~StaticMeshVertexInput() 
{
    
}

void StaticMeshVertexInput::setPositionStream(const VertexStreamComponent& positionStream) 
{
    declaration->addVertexStream(positionStream);
    positionDeclaration->addVertexStream(positionStream);
    data.positionStream = positionStream;
}

void StaticMeshVertexInput::setTangentXStream(const VertexStreamComponent& tangentXStream) 
{
    declaration->addVertexStream(tangentXStream);
    data.tangentBasisComponents[0] = tangentXStream;
}

void StaticMeshVertexInput::setTangentZStream(const VertexStreamComponent& tangentZStream) 
{
    declaration->addVertexStream(tangentZStream);
    data.tangentBasisComponents[1] = tangentZStream;
}

void StaticMeshVertexInput::setTexCoordStream(uint32 index, const VertexStreamComponent& textureStream) 
{
    //TODO: replace add with proper indexing
    declaration->addVertexStream(textureStream);
    data.textureCoordinates.add(textureStream);
}

void StaticMeshVertexInput::setColorStream(const VertexStreamComponent& colorStream)
{
    declaration->addVertexStream(colorStream);
    data.colorComponent = colorStream;
}

IMPLEMENT_VERTEX_INPUT_TYPE(StaticMeshVertexInput, "StaticMeshVertexInput");