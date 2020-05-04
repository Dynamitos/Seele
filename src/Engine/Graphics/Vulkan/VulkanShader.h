#pragma once
#include "VulkanGraphicsResources.h"
#include "VulkanGraphicsEnums.h"

namespace Seele
{
namespace Vulkan
{

class Shader
{
public:
    Shader(PGraphics graphics, ShaderType shaderType, VkShaderStageFlags stage);
    virtual ~Shader();

    VkShaderModule getModuleHandle() const
    {
        return module;
    }
    const char* getEntryPointName() const
    {
        return entryPointName.c_str();
    }
private:
    VkShaderModule module;
    std::string entryPointName;
};
DEFINE_REF(Shader);

template <typename Base, ShaderType shaderType, VkShaderStageFlags stage>
class ShaderBase : public Base, public Shader
{
public:
    ShaderBase(PGraphics graphics)
        : Shader(graphics, shaderType, stage)
    {
    }
    virtual ~ShaderBase()
    {
    }
};
typedef ShaderBase<Gfx::VertexShader, ShaderType::VERTEX, VK_SHADER_STAGE_VERTEX_BIT> VertexShader;
typedef ShaderBase<Gfx::ControlShader, ShaderType::CONTROL, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT> ControlShader;
typedef ShaderBase<Gfx::EvaluationShader, ShaderType::EVALUATION, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT> EvaluationShader;
typedef ShaderBase<Gfx::GeometryShader, ShaderType::GEOMETRY, VK_SHADER_STAGE_GEOMETRY_BIT> GeometryShader;
typedef ShaderBase<Gfx::FragmentShader, ShaderType::FRAGMENT, VK_SHADER_STAGE_FRAGMENT_BIT> FragmentShader;
typedef ShaderBase<Gfx::ComputeShader, ShaderType::COMPUTE, VK_SHADER_STAGE_COMPUTE_BIT> ComputeShader;

DEFINE_REF(VertexShader);
DEFINE_REF(ControlShader);
DEFINE_REF(EvaluationShader);
DEFINE_REF(GeometryShader);
DEFINE_REF(FragmentShader);
DEFINE_REF(ComputeShader);

} // namespace Vulkan
}