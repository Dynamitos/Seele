#pragma once
#include "GraphicsResources.h"

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
    void registerMaterial(PMaterialAsset material);
private:
    Array<PMaterialAsset> pendingCompiles;
    PGraphics graphics;
};
DEFINE_REF(ShaderCompiler)
} // namespace Gfx
} // namespace Seele