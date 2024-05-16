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
    char materialName[64];
    uint8 hasFragment;
    uint8 useMeshShading;
    uint8 hasTaskShader;
    uint8 useMaterial;
    uint8 positionOnly;
    //TODO: lightmapping etc
    ShaderPermutation()
    {
        std::memset(this, 0, sizeof(ShaderPermutation));
    }
    void setTaskFile(std::string_view name)
    {
        std::memset(taskFile, 0, sizeof(taskFile));
        hasTaskShader = 1;
        strncpy(taskFile, name.data(), sizeof(taskFile));
    }
    void setVertexFile(std::string_view name)
    {
        std::memset(vertexMeshFile, 0, sizeof(vertexMeshFile));
        useMeshShading = 0;
        strncpy(vertexMeshFile, name.data(), sizeof(vertexMeshFile));
    }
    void setMeshFile(std::string_view name)
    {
        std::memset(vertexMeshFile, 0, sizeof(vertexMeshFile));
        useMeshShading = 1;
        strncpy(vertexMeshFile, name.data(), sizeof(vertexMeshFile));
    }
    void setFragmentFile(std::string_view name)
    {
        std::memset(fragmentFile, 0, sizeof(fragmentFile));
        hasFragment = 1;
        strncpy(fragmentFile, name.data(), sizeof(fragmentFile));
    }
    void setVertexData(std::string_view name)
    {
        std::memset(vertexDataName, 0, sizeof(vertexDataName));
        strncpy(vertexDataName, name.data(), sizeof(vertexDataName));
    }
    void setMaterial(std::string_view name)
    {
        std::memset(materialName, 0, sizeof(materialName));
        useMaterial = 1;
        strncpy(materialName, name.data(), sizeof(materialName));
    }
    void setPositionOnly()
    {
        positionOnly = true;
    }
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
    OPipelineLayout pipelineLayout;
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
    void registerRenderPass(Gfx::PPipelineLayout baseLayout,
        std::string name,
        std::string mainFile,
        bool positionOnly = true,
        bool useMaterials = false,
        bool hasFragmentShader = false,
        std::string fragmentFile = "",
        bool useMeshShading = false,
        bool hasTaskShader = false,
        std::string taskFile = "");
private:
    void compile();
    void createShaders(ShaderPermutation permutation, OPipelineLayout layout);
    std::mutex shadersLock;
    Map<PermutationId, ShaderCollection> shaders;
    Map<std::string, PMaterial> materials;
    Map<std::string, VertexData*> vertexData;
    struct PassConfig
    {
        Gfx::PPipelineLayout baseLayout;
        std::string taskFile;
        std::string mainFile;
        std::string fragmentFile;
        bool hasFragmentShader;
        bool useMeshShading;
        bool hasTaskShader;
        bool useMaterial;
        bool positionOnly;
    };
    Map<std::string, PassConfig> passes;
    Gfx::PGraphics graphics;
};
DEFINE_REF(ShaderCompiler)
}
} // namespace Seele
