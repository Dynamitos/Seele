#pragma once
#include "Enums.h"
#include "Graphics/Shader.h"
#include "Resources.h"


namespace Seele {
namespace Vulkan {
DECLARE_REF(Graphics)
DECLARE_REF(DescriptorLayout)
class Shader {
  public:
    Shader(PGraphics graphics, VkShaderStageFlags stage);
    virtual ~Shader();

    void create(const ShaderCreateInfo& createInfo);
    void create(std::string_view binary);

    constexpr VkShaderModule getModuleHandle() const { return module; }
    constexpr const char* getEntryPointName() const {
        // SLang renames all entry points to main, so we dont need that
        return "main";// entryPointName.c_str();
    }
    constexpr VkShaderStageFlags getStage() const { return stage; }
    uint32 getShaderHash() const;

  private:
    PGraphics graphics;
    VkShaderModule module;
    VkShaderStageFlags stage;
    std::string entryPointName;
    uint32 hash;
};
DEFINE_REF(Shader)

template <typename Base, VkShaderStageFlags stageFlags> class ShaderBase : public Base, public Shader {
  public:
    ShaderBase(PGraphics graphics) : Shader(graphics, stageFlags) {}
    virtual ~ShaderBase() {}
};
using VertexShader = ShaderBase<Gfx::VertexShader, VK_SHADER_STAGE_VERTEX_BIT>;
using FragmentShader = ShaderBase<Gfx::FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT>;
using ComputeShader = ShaderBase<Gfx::ComputeShader, VK_SHADER_STAGE_COMPUTE_BIT>;
using TaskShader = ShaderBase<Gfx::TaskShader, VK_SHADER_STAGE_TASK_BIT_EXT>;
using MeshShader = ShaderBase<Gfx::MeshShader, VK_SHADER_STAGE_MESH_BIT_EXT>;

DEFINE_REF(VertexShader)
DEFINE_REF(FragmentShader)
DEFINE_REF(ComputeShader)
DEFINE_REF(TaskShader)
DEFINE_REF(MeshShader)

// Ray Tracing
using RayGenShader = ShaderBase<Gfx::RayGenShader, VK_SHADER_STAGE_RAYGEN_BIT_KHR>;
using AnyHitShader = ShaderBase<Gfx::AnyHitShader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR>;
using ClosestHitShader = ShaderBase<Gfx::ClosestHitShader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR>;
using MissShader = ShaderBase<Gfx::MissShader, VK_SHADER_STAGE_MISS_BIT_KHR>;
using IntersectionShader = ShaderBase<Gfx::IntersectionShader, VK_SHADER_STAGE_INTERSECTION_BIT_KHR>;
using CallableShader = ShaderBase<Gfx::CallableShader, VK_SHADER_STAGE_CALLABLE_BIT_KHR>;

DEFINE_REF(RayGenShader)
DEFINE_REF(AnyHitShader)
DEFINE_REF(ClosestHitShader)
DEFINE_REF(MissShader)
DEFINE_REF(IntersectionShader)
DEFINE_REF(CallableShader)
} // namespace Vulkan
} // namespace Seele