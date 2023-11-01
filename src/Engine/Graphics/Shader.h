#pragma once
#include "Enums.h"
#include "CRC.h"

namespace Seele
{
namespace Gfx
{

class Shader
{};
DEFINE_REF(Shader)
class TaskShader
{
public:
    TaskShader() {}
    virtual ~TaskShader() {}
};
DEFINE_REF(TaskShader)
class MeshShader
{
public:
    MeshShader() {}
    virtual ~MeshShader() {}
};
DEFINE_REF(MeshShader)
class VertexShader
{
public:
    VertexShader() {}
    virtual ~VertexShader() {}
};
DEFINE_REF(VertexShader)
class FragmentShader
{
public:
    FragmentShader() {}
    virtual ~FragmentShader() {}
};
DEFINE_REF(FragmentShader)

class ComputeShader
{
public:
    ComputeShader() {}
    virtual ~ComputeShader() {}
};
DEFINE_REF(ComputeShader)

    //Uniquely identifies a permutation of shaders
    //using the type parameters used to generate it
struct ShaderPermutation
{
    RenderPassType passType;
    char vertexInputName[15];
    char materialName[16];
    //TODO: lightmapping etc
};
//Hashed ShaderPermutation for fast lookup
struct PermutationId
{
    uint32 hash;
    PermutationId()
        : hash(0)
    {}
    PermutationId(ShaderPermutation permutation)
        : hash(CRC::Calculate(&permutation, sizeof(ShaderPermutation), CRC::CRC_32()))
    {}
    friend inline bool operator==(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash == rhs.hash;
    }
    friend inline bool operator!=(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash != rhs.hash;
    }
    friend inline bool operator<(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash < rhs.hash;
    }
    friend inline bool operator>(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash > rhs.hash;
    }
};
struct ShaderCollection
{
    PermutationId id;
    //PVertexDeclaration vertexDeclaration;
    PVertexShader vertexShader;
    PFragmentShader fragmentShader;
};
class ShaderMap
{
public:
    ShaderMap();
    ~ShaderMap();
    const ShaderCollection* findShaders(PermutationId&& id) const;
    ShaderCollection& createShaders(
        PGraphics graphics,
        RenderPassType passName,
        bool bPositionOnly);
private:
    std::mutex shadersLock;
    Array<ShaderCollection> shaders;
};
DEFINE_REF(ShaderMap)
}
} // namespace Seele