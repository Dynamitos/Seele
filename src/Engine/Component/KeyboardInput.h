#pragma once
#include "Graphics/Resources.h"

namespace Seele {
namespace Component {
struct KeyboardInput {
    Seele::StaticArray<bool, static_cast<size_t>(Seele::KeyCode::KEY_LAST)> keys;
    bool mouse1;
    bool mouse2;
    float mouseX;
    float mouseY;
    float deltaX;
    float deltaY;
    float scrollX;
    float scrollY;
};
} // namespace Component
} // namespace Seele