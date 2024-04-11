#pragma once
#include "Resources.h"
#include "Enums.h"
#include "Graphics/Shader.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Graphics)
DECLARE_REF(DescriptorLayout)
class Shader
{
public:
    Shader(PGraphics graphics, VkShaderStageFlags stage);
    virtual ~Shader();

    void create(const ShaderCreateInfo& createInfo);

    constexpr VkShaderModule getModuleHandle() const
    {
        return module;
    }
    constexpr const char* getEntryPointName() const
    {
        //SLang renames all entry points to main, so we dont need that
        return "main";//entryPointName.c_str();
    }
    constexpr VkShaderStageFlags getStage() const
    {
        return stage;
    }
    uint32 getShaderHash() const;
private:
    PGraphics graphics;
    VkShaderModule module;
    VkShaderStageFlags stage;
    uint32 hash;
};
DEFINE_REF(Shader)

template <typename Base, VkShaderStageFlags stageFlags>
class ShaderBase : public Base, public Shader
{
public:
    ShaderBase(PGraphics graphics)
        : Shader(graphics, stageFlags)
    {
    }
    virtual ~ShaderBase()
    {
    }
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
} // namespace Vulkan
}