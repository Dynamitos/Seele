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
    Shader(PGraphics graphics, ShaderType shaderType, VkShaderStageFlags stage);
    virtual ~Shader();

    void create(const ShaderCreateInfo& createInfo);

    VkShaderModule getModuleHandle() const
    {
        return module;
    }
    const char* getEntryPointName() const
    {
        //SLang renames all entry points to main, so we dont need that
        return "main";//entryPointName.c_str();
    }
    Map<uint32, PDescriptorLayout> getDescriptorLayouts();
    uint32 getShaderHash() const;
private:
    PGraphics graphics;
    Map<uint32, PDescriptorLayout> descriptorSets;
    VkShaderModule module;
    ShaderType type;
    VkShaderStageFlags stage;
    std::string entryPointName;
    uint32 hash;
};
DEFINE_REF(Shader)

template <typename Base, ShaderType shaderType, VkShaderStageFlags stageFlags>
class ShaderBase : public Base, public Shader
{
public:
    ShaderBase(PGraphics graphics)
        : Shader(graphics, shaderType, stageFlags)
    {
    }
    virtual ~ShaderBase()
    {
    }
};
typedef ShaderBase<Gfx::VertexShader, ShaderType::VERTEX, VK_SHADER_STAGE_VERTEX_BIT> VertexShader;
typedef ShaderBase<Gfx::FragmentShader, ShaderType::FRAGMENT, VK_SHADER_STAGE_FRAGMENT_BIT> FragmentShader;
typedef ShaderBase<Gfx::ComputeShader, ShaderType::COMPUTE, VK_SHADER_STAGE_COMPUTE_BIT> ComputeShader;
typedef ShaderBase<Gfx::TaskShader, ShaderType::TASK, VK_SHADER_STAGE_TASK_BIT_EXT> TaskShader;
typedef ShaderBase<Gfx::MeshShader, ShaderType::MESH, VK_SHADER_STAGE_MESH_BIT_EXT> MeshShader;

DEFINE_REF(VertexShader)
DEFINE_REF(FragmentShader)
DEFINE_REF(ComputeShader)
DEFINE_REF(TaskShader)
DEFINE_REF(MeshShader)

} // namespace Vulkan
}