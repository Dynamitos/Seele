#pragma once
#include "Graphics/Enums.h"
#include "Graphics/Shader.h"
#include "Resources.h"

namespace Seele {
namespace Metal {

class Shader {
  public:
    Shader(PGraphics graphics, Gfx::SeShaderStageFlags stage);
    virtual ~Shader();

    void create(const ShaderCreateInfo& createInfo);

    constexpr MTL::Function* getFunction() const { return function; }
    constexpr const char* getEntryPointName() const {
        // SLang renames all entry points to main, so we dont need that
        return "main"; // entryPointName.c_str();
    }
    uint32 getShaderHash() const;

  private:
    Gfx::SeShaderStageFlags stage;
    PGraphics graphics;
    MTL::Library* library;
    MTL::Function* function;
    // since metal is stupid and doesnt let us create argument encoders for
    // descriptors of stages that dont use them, we have to track
    // which stage uses what descriptors, which makes sense since functions are independent
    // but since metal ALSO doesnt provide any way to find out we have to
    // literally manually string search the generated code to find out
    Array<uint32> usedDescriptors;
    uint32 hash;
    friend class PipelineCache;
};
DEFINE_REF(Shader)

template <typename Base, Gfx::SeShaderStageFlags flags> class ShaderBase : public Base, public Shader {
  public:
    ShaderBase(PGraphics graphics) : Shader(graphics, flags) {}
    virtual ~ShaderBase() {}
};
using VertexShader = ShaderBase<Gfx::VertexShader, Gfx::SE_SHADER_STAGE_VERTEX_BIT>;
using FragmentShader = ShaderBase<Gfx::FragmentShader, Gfx::SE_SHADER_STAGE_FRAGMENT_BIT>;
using ComputeShader = ShaderBase<Gfx::ComputeShader, Gfx::SE_SHADER_STAGE_COMPUTE_BIT>;
using TaskShader = ShaderBase<Gfx::TaskShader, Gfx::SE_SHADER_STAGE_TASK_BIT_EXT>;
using MeshShader = ShaderBase<Gfx::MeshShader, Gfx::SE_SHADER_STAGE_MESH_BIT_EXT>;

using RayGenShader = ShaderBase<Gfx::RayGenShader, Gfx::SE_SHADER_STAGE_RAYGEN_BIT_KHR>;
using ClosestHitShader = ShaderBase<Gfx::ClosestHitShader, Gfx::SE_SHADER_STAGE_CLOSEST_HIT_BIT_KHR>;
using IntersectionShader = ShaderBase<Gfx::IntersectionShader, Gfx::SE_SHADER_STAGE_INTERSECTION_BIT_KHR>;
using AnyHitShader = ShaderBase<Gfx::AnyHitShader, Gfx::SE_SHADER_STAGE_ANY_HIT_BIT_KHR>;
using MissShader = ShaderBase<Gfx::MissShader, Gfx::SE_SHADER_STAGE_MISS_BIT_KHR>;
using CallableShader = ShaderBase<Gfx::CallableShader, Gfx::SE_SHADER_STAGE_CALLABLE_BIT_KHR>;

DEFINE_REF(VertexShader)
DEFINE_REF(FragmentShader)
DEFINE_REF(ComputeShader)
DEFINE_REF(TaskShader)
DEFINE_REF(MeshShader)

DEFINE_REF(RayGenShader)
DEFINE_REF(ClosestHitShader)
DEFINE_REF(IntersectionShader)
DEFINE_REF(AnyHitShader)
DEFINE_REF(MissShader)
DEFINE_REF(CallableShader)
} // namespace Metal
} // namespace Seele
