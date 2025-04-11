#pragma once
#include "Component.h"
#include "Math/Matrix.h"
#include "Transform.h"

namespace Seele {
namespace Component {
struct Camera {
    Matrix4 viewMatrix;
    bool mainCamera = false;
};
} // namespace Component
} // namespace Seele
