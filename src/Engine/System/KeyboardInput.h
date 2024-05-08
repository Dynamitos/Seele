#pragma once
#include "ComponentSystem.h"
#include "Component/KeyboardInput.h"

namespace Seele
{
namespace System
{
class KeyboardInput : public ComponentSystem<Component::KeyboardInput>
{
public:
    KeyboardInput(PScene scene);
    virtual ~KeyboardInput();
    virtual void update(Component::KeyboardInput& input) override;
    void keyCallback(KeyCode code, InputAction action, KeyModifier modifier);
    void mouseCallback(double xPos, double yPos);
    void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier);
    void scrollCallback(double xScroll, double yScroll);
private:
    Seele::StaticArray<bool, (size_t)KeyCode::KEY_LAST> keys;
    bool mouse1 = false;
    bool mouse2 = false;
    float lastMouseX = 0;
    float lastMouseY = 0;
    float mouseX = 0;
    float mouseY = 0;
    float deltaX = 0;
    float deltaY = 0;
    float scrollX = 0;
    float scrollY = 0;
};
DEFINE_REF(KeyboardInput)
}
}