#pragma once
#include "Material/Material.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Graphics)
namespace Gfx
{
class ShaderCompiler
{
public:
    ShaderCompiler(PGraphics graphics);
    ~ShaderCompiler();
private:
    PGraphics graphics;
};
DEFINE_REF(ShaderCompiler)
} // namespace Gfx
} // namespace Seele