#include "VertexShaderInput.h"
#include "Graphics/Mesh.h"
#include "Graphics/Graphics.h"
#include <sstream>
#include <memory>

using namespace Seele;

void Seele::Serialization::save(ArchiveBuffer& buffer, VertexStreamComponent& comp)
{
    if (comp.vertexBuffer == nullptr)
    {
        Serialization::save(buffer, (uint32)0);
    }
    else
    {
        Serialization::save(buffer, comp.vertexBuffer->getNumVertices());
        Serialization::save(buffer, comp.vertexBuffer->getVertexSize());
        Array<uint8> rawBuffer;
        comp.vertexBuffer->download(rawBuffer);
        Serialization::save(buffer, rawBuffer);
        Serialization::save(buffer, comp.streamOffset);
        Serialization::save(buffer, comp.offset);
        Serialization::save(buffer, comp.stride);
        Serialization::save(buffer, comp.type);
    }
}
void Seele::Serialization::load(ArchiveBuffer& buffer, VertexStreamComponent& comp)
{
    uint32 numVertices;
    Serialization::load(buffer, numVertices);
    if (numVertices == 0)
    {
        comp.vertexBuffer = nullptr;
    }
    else
    {
        uint32 vertexSize;
        Serialization::load(buffer, vertexSize);
        Array<uint8> rawBuffer;
        Serialization::load(buffer, rawBuffer);
        VertexBufferCreateInfo createInfo = {
            .resourceData = {
                .size = rawBuffer.size(),
                .data = rawBuffer.data(),
            },
            .vertexSize = vertexSize,
            .numVertices = numVertices,
        };
        comp.vertexBuffer = buffer.getGraphics()->createVertexBuffer(createInfo);
        Serialization::load(buffer, comp.streamOffset);
        Serialization::load(buffer, comp.offset);
        Serialization::load(buffer, comp.stride);
        Serialization::load(buffer, comp.type);
    }
}

List<VertexInputType*>& VertexInputType::getTypeList()
{
    static List<VertexInputType*> globalTypeList;
    return globalTypeList;
}

VertexInputType* VertexInputType::getVertexInputByName(const std::string& name)
{
    for(auto type : getTypeList())
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
    getTypeList().add(this);
}

VertexInputType::~VertexInputType() 
{
    getTypeList().remove(getTypeList().find(this));
}

const char* VertexInputType::getName() 
{
    return name;
}

std::string VertexInputType::getShaderFilename() 
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

void VertexShaderInput::init(Gfx::PGraphics) {}

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
    vertexStream.stride = static_cast<uint8>(component.stride);
    vertexStream.offset = static_cast<uint8>(component.offset);
    
    return Gfx::VertexElement((uint8)streams.indexOf(streams.addUnique(vertexStream)), static_cast<uint8>(component.offset), component.type, attributeIndex, vertexStream.stride);
}

Gfx::VertexElement VertexShaderInput::accessPositionStreamComponent(const VertexStreamComponent& component, uint8 attributeIndex) 
{
    VertexStream vertexStream;
    vertexStream.vertexBuffer = component.vertexBuffer;
    vertexStream.stride = static_cast<uint8>(component.stride);
    vertexStream.offset = static_cast<uint8>(component.offset);
    
    return Gfx::VertexElement((uint8)positionStreams.indexOf(positionStreams.addUnique(vertexStream)), static_cast<uint8>(component.offset), component.type, attributeIndex, vertexStream.stride);
}

void VertexShaderInput::initDeclaration(Gfx::PGraphics graphics, Array<Gfx::VertexElement>& elements) 
{
    declaration = graphics->createVertexDeclaration(elements);
}

void VertexShaderInput::initPositionDeclaration(Gfx::PGraphics graphics, const Array<Gfx::VertexElement>& elements) 
{
    positionDeclaration = graphics->createVertexDeclaration(elements);
}
