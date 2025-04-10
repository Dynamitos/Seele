#pragma once
#include "Graphics/Resources.h"

namespace Seele {
namespace Component {
struct KeyboardInput {
    Seele::StaticArray<bool, static_cast<size_t>(Seele::KeyCode::KEY_LAST)> keys;
    bool mouse1 = false;
    bool mouse2 = false;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    float deltaX = 0.0f;
    float deltaY = 0.0f;
    float scrollX = 0.0f;
    float scrollY = 0.0f;
};
} // namespace Component
} // namespace Seele