#pragma once
#include "Math/Vector.h"

namespace Seele {
namespace Component {
struct PointLight {
    Vector4 positionWS;
    // Vector4 positionVS;
    Vector4 colorRange;
};
} // namespace Component
} // namespace Seele
