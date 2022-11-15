#include "StaticMeshVertexInput.h"
#include "Graphics/Graphics.h"

using namespace Seele;

StaticMeshVertexInput::StaticMeshVertexInput(std::string name) 
    : VertexShaderInput(name)
{
    
}

StaticMeshVertexInput::~StaticMeshVertexInput() 
{
    
}

void StaticMeshVertexInput::init(Gfx::PGraphics graphics) 
{
    if(data.positionStream.vertexBuffer != data.tangentBasisComponents[0].vertexBuffer)
    {
        Array<Gfx::VertexElement> positionOnlyStreamElements;
        positionOnlyStreamElements.add(accessPositionStreamComponent(data.positionStream, 0));

        initPositionDeclaration(graphics, positionOnlyStreamElements);
    }

    Array<Gfx::VertexElement> elements;
    if(data.positionStream.vertexBuffer != nullptr)
    {
        elements.add(accessStreamComponent(data.positionStream, 0));
    }
    
    uint8 tangentBasisAttributes[3] = {1, 2, 3};
    for(int32 axisIndex = 0; axisIndex < 3; axisIndex++)
    {
        if(data.tangentBasisComponents[axisIndex].vertexBuffer != nullptr)
        {
            elements.add(accessStreamComponent(data.tangentBasisComponents[axisIndex], tangentBasisAttributes[axisIndex]));
        }
    }

    if(data.colorComponent.vertexBuffer != nullptr)
    {
        elements.add(accessStreamComponent(data.colorComponent, 4));
    }
    else
    {
        elements.add(accessStreamComponent(VertexStreamComponent(graphics->getNullVertexBuffer(), 0, 0, Gfx::SE_FORMAT_R32G32B32A32_SFLOAT), 4));
    }
    if(data.textureCoordinates.size())
    {
        const int32 baseTexCoordAttribute = 5;
        for(uint32 coordinateIndex = 0; coordinateIndex < data.textureCoordinates.size(); ++coordinateIndex)
        {
            elements.add(accessStreamComponent(
                data.textureCoordinates[coordinateIndex],
                static_cast<uint8>(baseTexCoordAttribute + coordinateIndex)
            ));
        }
    }
    initDeclaration(graphics, elements);
}

void StaticMeshVertexInput::setData(StaticMeshDataType&& _data) 
{
    data = std::move(_data);
}

IMPLEMENT_VERTEX_INPUT_TYPE(StaticMeshVertexInput, "StaticMeshVertexInput")