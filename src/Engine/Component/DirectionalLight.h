#pragma once
#include "Math/Vector.h"
namespace Seele {
namespace Component {
struct DirectionalLight {
    Vector4 colorIntensity;
    Vector4 direction;
};
} // namespace Component
} // namespace Seele
