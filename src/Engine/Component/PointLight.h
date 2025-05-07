#pragma once
#include "Math/Vector.h"

namespace Seele {
namespace Component {
struct PointLight {
    Vector color;
    float intensity;
    float attenuation;
};
} // namespace Component
} // namespace Seele
