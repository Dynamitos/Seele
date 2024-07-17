#pragma once
#include "Math/Vector.h"

namespace Seele {
namespace Component {
struct PointLight {
    // give the lights a radius so that they are not actual points
    Vector4 positionWSRadius;
    Vector4 colorRange;
};
} // namespace Component
} // namespace Seele
