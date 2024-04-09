#pragma once
#include "Graphics/Enums.h"
#include "Metal/MTLPixelFormat.hpp"
#include "Metal/MTLTexture.hpp"

namespace Seele
{
namespace Metal
{
MTL::PixelFormat cast(Gfx::SeFormat format);
Gfx::SeFormat cast(MTL::PixelFormat format);
}
}