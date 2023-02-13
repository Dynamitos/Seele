#pragma once
#include "VertexShaderInput.h"

namespace Seele
{
enum { MAX_TEXCOORDS = 4 };
struct StaticMeshDataType
{
    VertexStreamComponent positionStream;
    VertexStreamComponent tangentBasisComponents[3];
    
    //Dont forget these are 4 component vectors
    VertexStreamComponent textureCoordinates[4];

    VertexStreamComponent colorComponent;
};
class StaticMeshVertexInput : public VertexShaderInput
{
    DECLARE_VERTEX_INPUT_TYPE(StaticMeshVertexInput)
public:
    StaticMeshVertexInput(std::string name);
    virtual ~StaticMeshVertexInput();
    virtual void init(Gfx::PGraphics graphics) override;
    virtual void save(ArchiveBuffer& buffer) override;
    virtual void load(ArchiveBuffer& buffer) override;
    void setData(StaticMeshDataType&& data);
private:
    StaticMeshDataType data;
};
DEFINE_REF(StaticMeshVertexInput)
namespace Serialization
{
    static void save(ArchiveBuffer& buffer, StaticMeshDataType& type)
    {
        Serialization::save(buffer, type.positionStream);
        Serialization::save(buffer, type.tangentBasisComponents[0]);
        Serialization::save(buffer, type.tangentBasisComponents[1]);
        Serialization::save(buffer, type.tangentBasisComponents[2]);
        Serialization::save(buffer, type.textureCoordinates[0]);
        Serialization::save(buffer, type.textureCoordinates[1]);
        Serialization::save(buffer, type.textureCoordinates[2]);
        Serialization::save(buffer, type.textureCoordinates[3]);
        Serialization::save(buffer, type.colorComponent);
    }
    static void load(ArchiveBuffer& buffer, StaticMeshDataType& type)
    {
        Serialization::load(buffer, type.positionStream);
        Serialization::load(buffer, type.tangentBasisComponents[0]);
        Serialization::load(buffer, type.tangentBasisComponents[1]);
        Serialization::load(buffer, type.tangentBasisComponents[2]);
        Serialization::load(buffer, type.textureCoordinates[0]);
        Serialization::load(buffer, type.textureCoordinates[1]);
        Serialization::load(buffer, type.textureCoordinates[2]);
        Serialization::load(buffer, type.textureCoordinates[3]);
        Serialization::load(buffer, type.colorComponent);
    }
} // namespace Serialization
} // namespace Seele