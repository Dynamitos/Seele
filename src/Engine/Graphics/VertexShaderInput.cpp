#include "VertexShaderInput.h"
#include "Graphics/Mesh.h"
#include "Graphics/Graphics.h"
#include <sstream>
#include <memory>

using namespace Seele;

List<VertexInputType*> VertexInputType::globalTypeList;

List<VertexInputType*>& VertexInputType::getTypeList()
{
    return globalTypeList;
}

VertexInputType* VertexInputType::getVertexInputByName(const std::string& name)
{
    for(auto type : globalTypeList)
    {
        if(name.compare(type->getName()) == 0)
        {
            return type;
        }
    }
    return nullptr;
}

VertexInputType::VertexInputType(const char* name,
        const char* shaderFilename) 
    : name(name)
    , shaderFilename(shaderFilename)
{
    globalTypeList.add(this);
}

VertexInputType::~VertexInputType() 
{
    globalTypeList.remove(globalTypeList.find(this));
}

const char* VertexInputType::getName() 
{
    return name;
}

const char* VertexInputType::getShaderFilename() 
{
    return shaderFilename;
}

VertexShaderInput::VertexShaderInput(std::string name) 
    : name(name)
{
    declaration = new Gfx::VertexDeclaration();
    positionDeclaration = new Gfx::VertexDeclaration();
}

VertexShaderInput::~VertexShaderInput()
{
}

void VertexShaderInput::getStreams(VertexInputStreamArray& outVertexStreams) const
{
    for(uint32 i = 0; i < streams.size(); ++i)
    {
        const VertexStream& stream = streams[i];
        if(stream.vertexBuffer == nullptr)
        {
            outVertexStreams.add(VertexInputStream(i, 0, nullptr));
        }
        else
        {
            outVertexStreams.add(VertexInputStream(i, stream.offset, stream.vertexBuffer));
        }
    }
}

void VertexShaderInput::getPositionOnlyStream(VertexInputStreamArray& outVertexStreams) const
{
    for (uint32 i = 0; i < positionStreams.size(); ++i)
    {
        const VertexStream& stream = positionStreams[i];
        outVertexStreams.add(VertexInputStream(i, stream.offset, stream.vertexBuffer));
    }
}

Gfx::VertexElement VertexShaderInput::accessStreamComponent(const VertexStreamComponent& component, uint8 attributeIndex) 
{
    VertexStream vertexStream;
    vertexStream.vertexBuffer = component.vertexBuffer;
    vertexStream.stride = component.stride;
    vertexStream.offset = component.offset;
    
    return Gfx::VertexElement((uint8)streams.indexOf(streams.addUnique(vertexStream)), component.offset, component.type, attributeIndex, vertexStream.stride);
}

Gfx::VertexElement VertexShaderInput::accessPositionStreamComponent(const VertexStreamComponent& component, uint8 attributeIndex) 
{
    VertexStream vertexStream;
    vertexStream.vertexBuffer = component.vertexBuffer;
    vertexStream.stride = component.stride;
    vertexStream.offset = component.offset;
    
    return Gfx::VertexElement((uint8)positionStreams.indexOf(positionStreams.addUnique(vertexStream)), component.offset, component.type, attributeIndex, vertexStream.stride);
}

void VertexShaderInput::initDeclaration(Gfx::PGraphics graphics, Array<Gfx::VertexElement>& elements) 
{
    declaration = graphics->createVertexDeclaration(elements);
}

void VertexShaderInput::initPositionDeclaration(Gfx::PGraphics graphics, const Array<Gfx::VertexElement>& elements) 
{
    positionDeclaration = graphics->createVertexDeclaration(elements);
}