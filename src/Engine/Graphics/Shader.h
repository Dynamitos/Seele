#pragma once
#include "Enums.h"
#include "CRC.h"
#include "Resources.h"
#include "VertexData.h"

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
    char taskFile[32];
    char vertexMeshFile[32];
    char fragmentFile[32];
    char vertexDataName[32];
    char materialName[32];
    uint8 hasFragment : 1;
    uint8 useMeshShading : 1;
    uint8 hasTaskShader : 1;
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
    friend constexpr bool operator==(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash == rhs.hash;
    }
    friend constexpr auto operator<=>(const PermutationId& lhs, const PermutationId& rhs)
    {
        return lhs.hash <=> rhs.hash;
    }
};
struct ShaderCollection
{
    OVertexDeclaration vertexDeclaration;
    OVertexShader vertexShader;
    OTaskShader taskShader;
    OMeshShader meshShader;
    OFragmentShader fragmentShader;
};
class ShaderCompiler
{
public:
    ShaderCompiler(Gfx::PGraphics graphics);
    ~ShaderCompiler();
    const ShaderCollection* findShaders(PermutationId id) const;
    void registerMaterial(PMaterial material);
    void registerVertexData(VertexData* vertexData);
    void registerRenderPass(std::string name,
        std::string mainFile,
        bool useMaterials = false,
        bool hasFragmentShader = false,
        std::string fragmentFile = "",
        bool useMeshShading = false,
        bool hasTaskShader = false,
        std::string taskFile = "");
private:
    void compile();
    ShaderCollection& createShaders(ShaderPermutation permutation);
    std::mutex shadersLock;
    Map<PermutationId, ShaderCollection> shaders;
    Map<std::string, PMaterial> materials;
    Map<std::string, VertexData*> vertexData;
    struct PassConfig
    {
        std::string taskFile;
        std::string mainFile;
        std::string fragmentFile;
        bool hasFragmentShader;
        bool useMeshShading;
        bool hasTaskShader;
        bool useMaterial;
    };
    Map<std::string, PassConfig> passes;
    Gfx::PGraphics graphics;
};
DEFINE_REF(ShaderCompiler)
}
} // namespace Seele