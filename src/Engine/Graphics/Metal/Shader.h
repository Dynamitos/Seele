#pragma once
#include "Graphics/Shader.h"
#include "Resources.h"

namespace Seele {
namespace Metal {
class Shader {
public:
  Shader(PGraphics graphics);
  virtual ~Shader();

  void create(const ShaderCreateInfo &createInfo);

  constexpr MTL::Function *getFunction() const { return function; }
  constexpr const char *getEntryPointName() const {
    // SLang renames all entry points to main, so we dont need that
    return "main"; // entryPointName.c_str();
  }
  uint32 getShaderHash() const;

private:
  PGraphics graphics;
  MTL::Library* library;
  MTL::Function *function;
  uint32 hash;
};
DEFINE_REF(Shader)

template <typename Base> class ShaderBase : public Base, public Shader {
public:
  ShaderBase(PGraphics graphics) : Shader(graphics) {}
  virtual ~ShaderBase() {}
};
using VertexShader = ShaderBase<Gfx::VertexShader>;
using FragmentShader = ShaderBase<Gfx::FragmentShader>;
using ComputeShader = ShaderBase<Gfx::ComputeShader>;
using TaskShader = ShaderBase<Gfx::TaskShader>;
using MeshShader = ShaderBase<Gfx::MeshShader>;

DEFINE_REF(VertexShader)
DEFINE_REF(FragmentShader)
DEFINE_REF(ComputeShader)
DEFINE_REF(TaskShader)
DEFINE_REF(MeshShader)
} // namespace Metal
} // namespace Seele