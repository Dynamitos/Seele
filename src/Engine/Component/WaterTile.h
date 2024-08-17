#pragma once
#include "MinimalEngine.h"
#include "Math/Vector.h"

namespace Seele {
namespace Component
{
struct WaterTile
{
    IVector2 location;
    float height;
    constexpr static float DIMENSIONS = 10;
};
}
}