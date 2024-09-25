#pragma once
#include "Math/Vector.h"
#include "MinimalEngine.h"

namespace Seele {
namespace Component {
struct TerrainTile {
    IVector2 location;
    float height;
    constexpr static float DIMENSIONS = 10;
};
} // namespace Component
} // namespace Seele