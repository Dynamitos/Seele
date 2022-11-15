#pragma once
#include "SystemBase.h"
#include "Scene/Component/Camera.h"

namespace Seele
{
namespace System
{
class CameraSystem : public SystemBase<Component::Camera>
{
public:
    CameraSystem(entt::registry& registry) : SystemBase(registry) {}
    virtual ~CameraSystem() {}
    virtual void update(Component::Camera& component);
private:
};
} // namespace System
} // namespace Seele
