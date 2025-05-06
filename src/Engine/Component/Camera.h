#pragma once
#include "Component.h"
#include "Math/Matrix.h"
#include "Transform.h"

namespace Seele {
namespace Component {
struct Camera {
    float nearPlane = 0.001f;
    float farPlane = 10000.0f;
    bool mainCamera = false;
};
} // namespace Component
} // namespace Seele
