#pragma once
#include "Graphics/GraphicsResources.h"

namespace Seele
{
namespace Component
{
struct Skybox
{
    Gfx::PTextureCube day;
    Gfx::PTextureCube night;
    Gfx::PSamplerState sampler;
    Vector fogColor;
    float blendFactor;
};
} // namespace Component
} // namespace Seele
