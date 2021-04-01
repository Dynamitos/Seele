#pragma once
#include "GraphicsResources.h"

namespace Seele
{
DECLARE_REF(Material)
DECLARE_NAME_REF(Gfx, Graphics)
namespace Gfx
{
class ShaderCompiler
{
public:
    ShaderCompiler(PGraphics graphics);
    ~ShaderCompiler();
    void registerMaterial(PMaterial material);
private:
    Array<PMaterial> pendingCompiles;
    PGraphics graphics;
};
DEFINE_REF(ShaderCompiler)
} // namespace Gfx
} // namespace Seele