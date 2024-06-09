#pragma once
#include "Graphics/Texture.h"

namespace Seele {
namespace Component {
struct Skybox {
    Gfx::PTextureCube day;
    Gfx::PTextureCube night;
    Vector fogColor;
    float blendFactor;
};
} // namespace Component
} // namespace Seele
