#pragma once
#include "CRC.h"
#include "Enums.h"
#include "Resources.h"
#include "VertexData.h"


namespace Seele {
namespace Gfx {

class Shader {};
DEFINE_REF(Shader)
class TaskShader {
  public:
    TaskShader() {}
    virtual ~TaskShader() {}
};
DEFINE_REF(TaskShader)
class MeshShader {
  public:
    MeshShader() {}
    virtual ~MeshShader() {}
};
DEFINE_REF(MeshShader)
class VertexShader {
  public:
    VertexShader() {}
    virtual ~VertexShader() {}
};
DEFINE_REF(VertexShader)
class FragmentShader {
  public:
    FragmentShader() {}
    virtual ~FragmentShader() {}
};
DEFINE_REF(FragmentShader)

class ComputeShader {
  public:
    ComputeShader() {}
    virtual ~ComputeShader() {}
};
DEFINE_REF(ComputeShader)

// Ray Tracing shaders
class RayGenShader {
  public:
    RayGenShader() {}
    virtual ~RayGenShader() {}
};
DEFINE_REF(RayGenShader)

class AnyHitShader {
  public:
    AnyHitShader() {}
    virtual ~AnyHitShader() {}
};
DEFINE_REF(AnyHitShader)

class ClosestHitShader {
  public:
    ClosestHitShader() {}
    virtual ~ClosestHitShader() {}
};
DEFINE_REF(ClosestHitShader)

class MissShader {
  public:
    MissShader() {}
    virtual ~MissShader() {}
};
DEFINE_REF(MissShader)

class CallableShader {
  public:
    CallableShader() {}
    virtual ~CallableShader() {}
};
DEFINE_REF(CallableShader)

// Uniquely identifies a permutation of shaders
// using the type parameters used to generate it
struct ShaderPermutation {
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
    uint8 depthCulling;
    uint8 visibilityPass;
    // TODO: lightmapping etc
    ShaderPermutation() { std::memset(this, 0, sizeof(ShaderPermutation)); }
    void setTaskFile(std::string_view name) {
        std::memset(taskFile, 0, sizeof(taskFile));
        hasTaskShader = 1;
        strncpy(taskFile, name.data(), sizeof(taskFile));
    }
    void setVertexFile(std::string_view name) {
        std::memset(vertexMeshFile, 0, sizeof(vertexMeshFile));
        useMeshShading = 0;
        strncpy(vertexMeshFile, name.data(), sizeof(vertexMeshFile));
    }
    void setMeshFile(std::string_view name) {
        std::memset(vertexMeshFile, 0, sizeof(vertexMeshFile));
        useMeshShading = 1;
        strncpy(vertexMeshFile, name.data(), sizeof(vertexMeshFile));
    }
    void setFragmentFile(std::string_view name) {
        std::memset(fragmentFile, 0, sizeof(fragmentFile));
        hasFragment = 1;
        strncpy(fragmentFile, name.data(), sizeof(fragmentFile));
    }
    void setVertexData(std::string_view name) {
        std::memset(vertexDataName, 0, sizeof(vertexDataName));
        strncpy(vertexDataName, name.data(), sizeof(vertexDataName));
    }
    void setMaterial(std::string_view name) {
        std::memset(materialName, 0, sizeof(materialName));
        useMaterial = 1;
        strncpy(materialName, name.data(), sizeof(materialName));
    }
    void setPositionOnly(bool enable) { positionOnly = enable; }
    void setDepthCulling(bool enable) { depthCulling = enable; }
    void setVisibilityPass(bool enable) { visibilityPass = enable; }
};
// Hashed ShaderPermutation for fast lookup
struct PermutationId {
    uint32 hash;
    PermutationId() : hash(0) {}
    PermutationId(ShaderPermutation permutation) : hash(CRC::Calculate(&permutation, sizeof(ShaderPermutation), CRC::CRC_32())) {}
    friend constexpr bool operator==(const PermutationId& lhs, const PermutationId& rhs) { return lhs.hash == rhs.hash; }
    friend constexpr auto operator<=>(const PermutationId& lhs, const PermutationId& rhs) { return lhs.hash <=> rhs.hash; }
};
struct ShaderCollection {
    OPipelineLayout pipelineLayout;
    OVertexShader vertexShader;
    OTaskShader taskShader;
    OMeshShader meshShader;
    OFragmentShader fragmentShader;
};

struct PassConfig {
    Gfx::PPipelineLayout baseLayout;
    std::string taskFile = "";
    std::string mainFile = "";
    std::string fragmentFile = "";
    bool hasFragmentShader = false;
    bool useMeshShading = false;
    bool hasTaskShader = false;
    bool useMaterial = false;
    bool useVisibility = false;
};
class ShaderCompiler {
  public:
    ShaderCompiler(Gfx::PGraphics graphics);
    ~ShaderCompiler();
    const ShaderCollection* findShaders(PermutationId id) const;
    void registerMaterial(PMaterial material);
    void registerVertexData(VertexData* vertexData);
    void registerRenderPass(std::string name, PassConfig config);
    ShaderPermutation getTemplate(std::string name);

  private:
    void compile();
    void createShaders(ShaderPermutation permutation, OPipelineLayout layout);
    std::mutex shadersLock;
    Map<PermutationId, ShaderCollection> shaders;
    Map<std::string, PMaterial> materials;
    Map<std::string, VertexData*> vertexData;
    Map<std::string, PassConfig> passes;
    Gfx::PGraphics graphics;
};
DEFINE_REF(ShaderCompiler)
} // namespace Gfx
} // namespace Seele
