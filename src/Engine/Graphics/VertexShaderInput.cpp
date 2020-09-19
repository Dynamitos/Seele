#include "VertexShaderInput.h"
#include "Graphics/Mesh.h"
#include <sstream>
#include <memory>

using namespace Seele;

List<VertexInputType*> VertexInputType::globalTypeList;

List<VertexInputType*> VertexInputType::getTypeList()
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
        const VertexInputStream& stream = streams[i];
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
        const VertexInputStream& stream = positionStreams[i];
        outVertexStreams.add(VertexInputStream(i, stream.offset, stream.vertexBuffer));
    }
}