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

  void create(const ShaderCreateInfo &createInfo);

  constexpr MTL::Function *getFunction() const { return function; }
  constexpr const char *getEntryPointName() const {
    // SLang renames all entry points to main, so we dont need that
    return "main"; // entryPointName.c_str();
  }
  uint32 getShaderHash() const;

private:
  Gfx::SeShaderStageFlags stage;
  PGraphics graphics;
  MTL::Library* library;
  MTL::Function *function;
  uint32 hash;
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

DEFINE_REF(VertexShader)
DEFINE_REF(FragmentShader)
DEFINE_REF(ComputeShader)
DEFINE_REF(TaskShader)
DEFINE_REF(MeshShader)
} // namespace Metal
} // namespace Seele