#include "KeyboardInput.h"

using namespace Seele;
using namespace Seele::System;

KeyboardInput::KeyboardInput(PScene scene) : ComponentSystem<Component::KeyboardInput>(scene) { std::memset(keys.data(), 0, sizeof(keys)); }

KeyboardInput::~KeyboardInput() {}

void KeyboardInput::update(Component::KeyboardInput& input) {
    std::memcpy(input.keys.data(), keys.data(), sizeof(keys));
    input.deltaX = deltaX;
    input.deltaY = deltaY;
    input.mouseX = mouseX;
    input.mouseY = mouseY;
    input.mouse1 = mouse1;
    input.mouse2 = mouse2;
    input.scrollX = scrollX;
    input.scrollY = scrollY;
    deltaX = mouseX - lastMouseX;
    deltaY = mouseY - lastMouseY;
    lastMouseX = mouseX;
    lastMouseY = mouseY;
    scrollX = 0;
    scrollY = 0;
}

void KeyboardInput::keyCallback(KeyCode code, InputAction action, KeyModifier) { keys[code] = action != InputAction::RELEASE; }

void KeyboardInput::mouseCallback(double x, double y) {
    mouseX = (float)x;
    mouseY = (float)y;
}

void KeyboardInput::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier) {
    if (button == MouseButton::MOUSE_BUTTON_1) {
        mouse1 = action != InputAction::RELEASE;
    }
    if (button == MouseButton::MOUSE_BUTTON_2) {
        mouse2 = action != InputAction::RELEASE;
    }
}

void KeyboardInput::scrollCallback(double xScroll, double yScroll) {
    scrollX = (float)xScroll;
    scrollY = (float)yScroll;
}
